#ifndef BUSYSPIN_WAIT_STRATEGY_HPP_
#define BUSYSPIN_WAIT_STRATEGY_HPP_

#include <sys/time.h>

#include "macro.hpp"
#include "sequence_barrier.hpp"
#include "sequence_group.hpp"

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

	long waitFor(const long& sequence, const Sequence& cursor,
			const SequenceGroup& dependents, const AlertableBarrier& barrier,
			const int64_t& timeout_micros) {
		struct timeval start_time, end_time;
		gettimeofday(&start_time, NULL);
		int64_t start_micro = start_time.tv_sec * 1000000 + start_time.tv_usec;
		int64_t available_sequence = 0;

		if (dependents.isEmpty()) {
			while ((available_sequence = cursor.get()) < sequence) {
				barrier.checkAlert();
				gettimeofday(&end_time, NULL);
				int64_t end_micro = end_time.tv_sec * 1000000
						+ end_time.tv_usec;
				if (timeout_micros < (end_micro - start_micro))
					break;
			}
		} else {
			while ((available_sequence = dependents.get()) < sequence) {
				barrier.checkAlert();
				gettimeofday(&end_time, NULL);
				int64_t end_micro = end_time.tv_sec * 1000000
						+ end_time.tv_usec;
				if (timeout_micros < (end_micro - start_micro))
					break;
			}
		}

		return available_sequence;
	}

	void signalAllWhenBlocking() {

	}

	DISALLOW_COPY_ASSIGN_MOVE(BusySpinStrategy)	;
};


#endif /* BUSYSPIN_WAIT_STRATEGY_HPP_ */
