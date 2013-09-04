#ifndef SINGLE_PRODUCER_SEQUENCER_HPP_
#define SINGLE_PRODUCER_SEQUENCER_HPP_

#include <cassert>

#include "abstract_sequencer.hpp"
#include "macro.hpp"

INTERNAL_NAMESPACE_BEGIN

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

	bool hasAvailableCapacity(int requiredCapacity) {
		long nextValue = pad.nextValue;
		long wrapPoint = (nextValue + requiredCapacity) - bufferSize;
		long cachedGatingSequence = pad.cachedValue;
		SequenceGroup *gatingSeq = &gatingSequences;

		if (wrapPoint > cachedGatingSequence || cachedGatingSequence > nextValue) {
			long minSequence = gatingSeq->getMinimumSequence(nextValue);
			pad.cachedValue = minSequence;
			if (wrapPoint > minSequence) {
				return false;
			}
		}

		return true;
	}

	long next(int n = 1) {
		assert(n > 0);

		long nextValue = pad.nextValue;
		long nextSequence = nextValue + n;
		long wrapPoint = nextSequence - bufferSize;
		long cachedGatingSequence = pad.cachedValue;
		SequenceGroup *gatingSeq = &gatingSequences;

		if (wrapPoint > cachedGatingSequence || cachedGatingSequence > nextValue) {
			long minSequence;
			while (wrapPoint > (minSequence = gatingSeq->getMinimumSequence(nextValue))) {
				std::this_thread::yield();
			}
			pad.cachedValue = minSequence;
		}

		pad.nextValue = nextSequence;
		return nextSequence;
	}

	long tryNext(int n = 1) {
		assert(n > 0);
		if (!hasAvailableCapacity(n)) {
			throw std::out_of_range("Insufficient capacity");
		}
		long nextSequence = pad.nextValue += n;
		return nextSequence;
	}

	long remainingCapacity() {
		long nextValue = pad.nextValue;
		long consumed = gatingSequences.getMinimumSequence(nextValue);
		long produced = nextValue;
		return getBufferSize() - (produced - consumed);
	}

	void claim(long sequence) {
		pad.nextValue = sequence;
	}

	void publish(long sequence) {
		m_cursor.set(sequence);
	}

	void publish(long lo, long hi) {
		publish(hi);
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

	long getHighestPublishedSequence(long lowerBound, long availableSequence) {
		return availableSequence;
	}

private:
	Padding pad;
};

INTERNAL_NAMESPACE_END

#endif /* SINGLE_PRODUCER_SEQUENCER_HPP_ */
