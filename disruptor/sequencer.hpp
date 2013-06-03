#ifndef SEQUENCER_HPP_
#define SEQUENCER_HPP_

#include <thread>

#include "sequence.hpp"
#include "sequence_group.hpp"
#include "utils.h"

/*
class Sequencer {
public:
	static const long INITIAL_CURSOR_VALUE = -1;
	int getBufferSize();
	bool hasAvailableCapacity();
	long next();
	long tryNext();
	long remainingCapacity();
	void claim(long sequence);
};
*/

class AbstractSequencer {
public:

	AbstractSequencer(int buffersize) : bufferSize(buffersize) {

	}

	int getBufferSize() {
		return bufferSize;
	}

	Sequence& getCursorSequence() {
		return m_cursor;
	}

	template<typename Collection>
	void addGatingSequences(Collection&& sequences) {
		gatingSequences.assign(std::forward<Collection>(sequences));
	}

	long getMinimumSequence() {
		return Util::getMinimumSequence(gatingSequences.begin(), gatingSequences.end(), m_cursor.get());
	}

	const SequenceGroup& getGatingSequences() const {
		return gatingSequences;
	}

protected:
	SequenceGroup gatingSequences;
	Sequence m_cursor;
	int bufferSize;

	DISALLOW_COPY_ASSIGN_MOVE(AbstractSequencer);
};

class SingleProducerSequencer : public AbstractSequencer {
private:
	struct Padding {
		long nextValue = Sequence::INITIAL_VALUE;
		long cachedValue = Sequence::INITIAL_VALUE;
		long pad[7];
	};

public:

	SingleProducerSequencer(int bufferSize) :
			AbstractSequencer(bufferSize) {
		//
	}

	long getNextValue() {
		return pad.nextValue;
	}

	bool hasAvailableCapacity(const SequenceGroup& gatingSequences, int requiredCapacity) {
		long nextValue = pad.nextValue;
		long wrapPoint = (nextValue + requiredCapacity) - bufferSize;
		long cachedGatingSequence = pad.cachedValue;

		if (wrapPoint > cachedGatingSequence || cachedGatingSequence > nextValue) {
			long minSequence = Util::getMinimumSequence(gatingSequences.begin(), gatingSequences.end(), nextValue);
			pad.cachedValue = minSequence;
			if (wrapPoint > minSequence) {
				return false;
			}
		}

		return true;
	}

	long next() {
		long nextValue = pad.nextValue;
		long nextSequence = nextValue + 1;
		long wrapPoint = nextSequence - bufferSize;
		long cachedGatingSequence = pad.cachedValue;

		if (wrapPoint > cachedGatingSequence || cachedGatingSequence > nextValue) {
			long minSequence;
			while (wrapPoint > (minSequence = Util::getMinimumSequence(gatingSequences.begin(), gatingSequences.end(), nextValue))) {
				std::this_thread::yield();
			}
			pad.cachedValue = minSequence;
		}

		pad.nextValue = nextSequence;
		return nextSequence;
	}

	long tryNext() {
		if (!hasAvailableCapacity(gatingSequences, (int)1)) {
			throw std::runtime_error("Try failed");
		}
		long nextSequence = ++pad.nextValue;
		return nextSequence;
	}

	long remainingCapacity() {
		long nextValue = pad.nextValue;
		long consumed = Util::getMinimumSequence(gatingSequences.begin(), gatingSequences.end(),nextValue);
		long produced = nextValue;
		return getBufferSize() - (produced - consumed);
	}

	void claim(long sequence) {
		pad.nextValue = sequence;
	}

	template<class WaitStrategy>
	void publish(long sequence, WaitStrategy& waitStrategy) {
		m_cursor.set(sequence);
		waitStrategy.signalAllWhenBlocking();
	}

	void ensureAvailable(long sequence) {
		//
	}

	bool isAvailable(long sequence) const {
		return sequence <= m_cursor.get();
	}

	const Sequence& getCursorSequence() const {
		return m_cursor;
	}

	Sequence& getCursorSequence() {
		return m_cursor;
	}

private:
	Padding pad;
};

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

	long next() {
		return next(1);
	}

	long next(int n) {
		if (n < 1) {
			throw std::runtime_error("n must be > 0");
		}

		long current;
		long next;

		do {
			current = m_cursor.get();
			next = current + n;

			long wrapPoint = next - bufferSize;
			long cachedGatingSequence = m_gatingSequenceCache.get();

			if (wrapPoint > cachedGatingSequence || cachedGatingSequence > current) {
				long gatingSequence = Util::getMinimumSequence(gatingSequences.begin(), gatingSequences.end(), current);
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

	long tryNext() {
		return tryNext(1);
	}

	long tryNext(int n) {
		if (n < 1) {
			throw std::runtime_error("n must be > 0");
		}

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
		long consumed = Util::getMinimumSequence(gatingSequences.begin(), gatingSequences.end(), m_cursor.get());
		long produced = m_cursor.get();
		return getBufferSize() - (produced - consumed);
	}

	void initialiseAvailableBuffer() {
		for (int i = bufferSize - 1; i != 0; i--) {
			setAvailableBufferValue(i, -1);
		}
		setAvailableBufferValue(0, -1);
	}

	template<class WaitStrategy>
	void publish(long sequence, WaitStrategy& waitStrategy) {
		setAvailable(sequence);
		waitStrategy.signalAllWhenBlocking();
	}
	template<class WaitStrategy>
	void publish(long lo, long hi, WaitStrategy& waitStrategy) {
		for (long l = lo; l <= hi; l++) {
			setAvailable(l);
		}
		waitStrategy.signalAllWhenBlocking();
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
			long minSequence = Util::getMinimumSequence(gatingSequences.begin(), gatingSequences.end(), cursorValue);
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


#endif /* SEQUENCER_HPP_ */
