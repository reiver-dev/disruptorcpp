#ifndef BUSYSPIN_WAIT_STRATEGY_HPP_
#define BUSYSPIN_WAIT_STRATEGY_HPP_

#include "macro.hpp"
#include "sequence_barrier.hpp"
#include "sequence_group.hpp"

INTERNAL_NAMESPACE_BEGIN

class BusySpinStrategy {
public:

	BusySpinStrategy() {
	}

	long waitFor(const long& sequence, const Sequence& cursor,
			const SequenceGroup& dependents, const AlertableBarrier& barrier) {
		long available_sequence = 0;
		if (dependents.isEmpty()) {
			while ((available_sequence = cursor.get()) < sequence) {
				barrier.checkAlert();
			}
		} else {
			while ((available_sequence = dependents.get()) < sequence) {
				barrier.checkAlert();
			}
		}
		return available_sequence;
	}

	void signalAllWhenBlocking() {

	}

	DISALLOW_COPY_MOVE(BusySpinStrategy);
};


INTERNAL_NAMESPACE_END

#endif /* BUSYSPIN_WAIT_STRATEGY_HPP_ */
