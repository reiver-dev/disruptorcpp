#ifndef WAIT_STRATEGY_HPP_
#define WAIT_STRATEGY_HPP_

#include <sys/time.h>
#include <mutex>
#include <condition_variable>

#include "sequence.hpp"
#include "sequence_barrier.hpp"

class BlockingWaitStrategy {
public:

	long waitFor(long sequence, const Sequence& cursorSequence,
			const SequenceGroup& dependentSequence,
			const AlertableBarrier& barrier) {
		long availableSequence;
		if ((availableSequence = cursorSequence.get()) < sequence) {
			std::unique_lock<std::mutex> ulock(m_mutex);
			while ((availableSequence = cursorSequence.get()) < sequence) {
				barrier.checkAlert();
				m_condition.wait(ulock);
			}
		}

		if (!dependentSequence.isEmpty())
			while ((availableSequence = dependentSequence.get()) < sequence) {
				barrier.checkAlert();
			}

		return availableSequence;
	}

	long waitFor(long sequence, const Sequence& cursorSequence,
			const SequenceGroup& dependentSequence,
			const AlertableBarrier& barrier, long micros) {
		long availableSequence;
		if ((availableSequence = cursorSequence.get()) < sequence) {
			std::unique_lock<std::mutex> ulock(m_mutex);
			while ((availableSequence = cursorSequence.get()) < sequence) {
				barrier.checkAlert();
				std::cv_status status = m_condition.wait_for(ulock,
						std::chrono::microseconds(micros));
				if (status == std::cv_status::timeout) {
					break;
				}

			}
		}

		if (!dependentSequence.isEmpty())
			while ((availableSequence = dependentSequence.get()) < sequence) {
				barrier.checkAlert();
			}

		return availableSequence;
	}

	void signalAllWhenBlocking() {
		//std::unique_lock<std::mutex> ulock(m_mutex);
		m_condition.notify_all();
	}

private:
	std::mutex m_mutex;
	std::condition_variable m_condition;
};

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

#endif /* WAIT_STRATEGY_HPP_ */
