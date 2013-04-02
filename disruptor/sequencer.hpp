#ifndef SEQUENCER_HPP_
#define SEQUENCER_HPP_

#include <thread>

#include "sequence.hpp"
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

class SingleProducerSequencer {
private:
    struct Padding {
    	long nextValue = Sequence::INITIAL_VALUE;
    	long cachedValue = Sequence::INITIAL_VALUE;
    	long pad[7];
    };

public:

	SingleProducerSequencer(int bufferSize) :
			bufferSize(bufferSize) {
		//
	}

	int getBufferSize() {
		return bufferSize;
	}

	long getNextValue() {
		return pad.nextValue;
	}

	bool hasAvailableCapacity(const std::vector<Sequence*>& gatingSequences,
			int requiredCapacity) {
		long nextValue = pad.nextValue;
		long wrapPoint = (nextValue + requiredCapacity) - bufferSize;
		long cachedGatingSequence = pad.cachedValue;

		if (wrapPoint > cachedGatingSequence
				|| cachedGatingSequence > nextValue) {
			long minSequence = Util::getMinimumSequence(gatingSequences, nextValue);
			pad.cachedValue = minSequence;

			if (wrapPoint > minSequence) {
				return false;
			}
		}

		return true;
	}

	long next(const SequenceGroup& gatingSequences) {
		long nextValue = pad.nextValue;

		long nextSequence = nextValue + 1;
		long wrapPoint = nextSequence - bufferSize;
		long cachedGatingSequence = pad.cachedValue;

		if (wrapPoint > cachedGatingSequence
				|| cachedGatingSequence > nextValue) {
			long minSequence;
			while (wrapPoint
					> (minSequence = Util::getMinimumSequence(gatingSequences.data(),
							nextValue))) {
				std::this_thread::yield();
			}
			pad.cachedValue = minSequence;
		}

		pad.nextValue = nextSequence;

		return nextSequence;
	}

	long tryNext(const std::vector<Sequence*>& gatingSequences) {
		if (!hasAvailableCapacity(gatingSequences, 1)) {
			throw std::runtime_error("Try failed");
		}

		long nextSequence = ++pad.nextValue;

		return nextSequence;
	}

	long remainingCapacity(std::vector<Sequence*>& gatingSequences) {
		long nextValue = pad.nextValue;

		long consumed = Util::getMinimumSequence(gatingSequences, nextValue);
		long produced = nextValue;
		return getBufferSize() - (produced - consumed);
	}

	void claim(long sequence) {
		pad.nextValue = sequence;
	}


private:
    int bufferSize;
    Padding pad;
};


#endif /* SEQUENCER_HPP_ */
