#ifndef MULTI_PRODUCER_SEQUENCER_HPP_
#define MULTI_PRODUCER_SEQUENCER_HPP_

#include <assert.h>
#include "abstract_sequencer.hpp"
#include "utils.hpp"
#include "macro.hpp"

INTERNAL_NAMESPACE_BEGIN

class MultiProducerSequencer : AbstractSequencer {
public:

	MultiProducerSequencer(int buffersize)
		: AbstractSequencer(buffersize)
		, availableBuffer(new int[bufferSize])
		, indexMask(bufferSize - 1)
		, indexShift(Util::log2(bufferSize)) {

	}

	~MultiProducerSequencer() {
		delete[] availableBuffer;
	}

	bool hasAvailableCapacity(int requiredCapacity) {
		return hasAvailableCapacity(gatingSequences, requiredCapacity, m_cursor.get());
	}

	void claim(long sequence) {
		m_cursor.set(sequence);
	}

	long next(int n = 1) {
		assert(n > 0);

		long current;
		long next;

		do {
			current = m_cursor.get();
			next = current + n;

			long wrapPoint = next - bufferSize;
			long cachedGatingSequence = m_gatingSequenceCache.get();

			if (wrapPoint > cachedGatingSequence || cachedGatingSequence > current) {
				long gatingSequence = gatingSequences.getMinimumSequence(current);
				if (wrapPoint > gatingSequence) {
					std::this_thread::yield(); // TODO, should we spin based on the wait strategy?
					continue;
				}
				m_gatingSequenceCache.set(gatingSequence);
			} else if (m_cursor.compareAndSet(current, next)) {
				break;
			}
		} while (true);

		return next;
	}

	long tryNext(int n = 1) {
		assert(n > 0);

		long current;
		long next;

		do {
			current = m_cursor.get();
			next = current + n;
			if (!hasAvailableCapacity(gatingSequences, n, current)) {
				throw std::runtime_error("insufficient capacity");
			}
		} while (!m_cursor.compareAndSet(current, next));

		return next;
	}

	long remainingCapacity() {
		long consumed = gatingSequences.getMinimumSequence(m_cursor.get());
		long produced = m_cursor.get();
		return getBufferSize() - (produced - consumed);
	}

	void initialiseAvailableBuffer() {
		for (int i = bufferSize - 1; i != 0; i--) {
			setAvailableBufferValue(i, -1);
		}
		setAvailableBufferValue(0, -1);
	}

	void publish(long sequence) {
		setAvailable(sequence);
	}

	void publish(long lo, long hi) {
		for (long l = lo; l <= hi; l++) {
			setAvailable(l);
		}
	}

	long getHighestPublishedSequence(long lowerBound, long availableSequence) {
		for (long sequence = lowerBound; sequence <= availableSequence; sequence++) {
			if (!isAvailable(sequence)) {
				return sequence - 1;
			}
		}
		return availableSequence;
	}

private:

	bool hasAvailableCapacity(const SequenceGroup& gatingSequences, int requiredCapacity, long cursorValue) {
		long wrapPoint = (cursorValue + requiredCapacity) - bufferSize;
		long cachedGatingSequence = m_gatingSequenceCache.get();

		if (wrapPoint > cachedGatingSequence || cachedGatingSequence > cursorValue) {
			long minSequence = gatingSequences.getMinimumSequence(cursorValue);
			m_gatingSequenceCache.set(minSequence);
			if (wrapPoint > minSequence) {
				return false;
			}
		}
		return true;
	}

	void setAvailable(long sequence) {
		setAvailableBufferValue(calculateIndex(sequence), calculateAvailabilityFlag(sequence));
	}

	void setAvailableBufferValue(int index, int flag) {
		std::atomic<int> *target = reinterpret_cast<std::atomic<int>*>(&availableBuffer[index]);
		target->store(flag);
		Util::arraySetAtomic(availableBuffer, index, flag);
	}

	bool isAvailable(long sequence) {
		int index = calculateIndex(sequence);
		int flag = calculateAvailabilityFlag(sequence);
		return Util::arrayGetAtomic(availableBuffer, index) == flag;
	}

	int calculateAvailabilityFlag(long sequence) {
		return (int) ((unsigned long) sequence >> indexShift);
	}

	int calculateIndex(long sequence) {
		return ((int) sequence) & indexMask;
	}

	Sequence m_gatingSequenceCache;

	int *availableBuffer;
	int indexMask;
	int indexShift;

};

INTERNAL_NAMESPACE_END


#endif /* MULTI_PRODUCER_SEQUENCER_HPP_ */
