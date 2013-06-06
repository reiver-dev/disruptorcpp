#ifndef BLOCKING_WAIT_STRATEGY_HPP_
#define BLOCKING_WAIT_STRATEGY_HPP_

#include <mutex>
#include <condition_variable>

#include "macro.hpp"
#include "sequence_barrier.hpp"
#include "sequence_group.hpp"

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


#endif /* BLOCKING_WAIT_STRATEGY_HPP_ */
