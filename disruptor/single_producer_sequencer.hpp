#ifndef SINGLE_PRODUCER_SEQUENCER_HPP_
#define SINGLE_PRODUCER_SEQUENCER_HPP_

#include "abstract_sequencer.hpp"

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
			long minSequence = gatingSequences.getMinimumSequence(nextValue);
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
			while (wrapPoint > (minSequence = gatingSequences.getMinimumSequence(nextValue))) {
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
		long consumed = gatingSequences.getMinimumSequence(nextValue);
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



#endif /* SINGLE_PRODUCER_SEQUENCER_HPP_ */
