#ifndef YIELD_WAIT_STRATEGY_HPP_
#define YIELD_WAIT_STRATEGY_HPP_

#include "macro.hpp"
#include "sequence_barrier.hpp"
#include "sequence_group.hpp"

class YieldStrategy {
public:

	YieldStrategy() {
	}

	long waitFor(const long& sequence, const Sequence& cursor,
			const SequenceGroup& dependents, const AlertableBarrier& barrier) {

		int counter = SPIN_TRIES;
		long available_sequence;

		if (dependents.isEmpty()) {
			while ((available_sequence = cursor.get()) < sequence) {
				applyWaitMethod(barrier, counter);
			}
		} else {
			while ((available_sequence = dependents.get()) < sequence) {
				applyWaitMethod(barrier, counter);
			}
		}
		return available_sequence;
	}

	void signalAllWhenBlocking() {

	}

private:

	static const int SPIN_TRIES = 100;

	void applyWaitMethod(const AlertableBarrier& barrier, int& counter) {
		barrier.checkAlert();
		if (!counter) {
			std::this_thread::yield();
		} else {
			--counter;
		}
	}

	DISALLOW_COPY_ASSIGN_MOVE(YieldStrategy);
};


#endif /* YIELD_WAIT_STRATEGY_HPP_ */
