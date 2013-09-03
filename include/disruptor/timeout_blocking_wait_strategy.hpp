#ifndef TIMEOUT_BLOCKING_WAIT_STRATEGY_HPP_
#define TIMEOUT_BLOCKING_WAIT_STRATEGY_HPP_

#include <mutex>
#include <condition_variable>

#include "macro.hpp"
#include "sequence_barrier.hpp"
#include "sequence_group.hpp"

INTERNAL_NAMESPACE_BEGIN

class TimeoutBlockingWaitStrategy {
public:

	long waitFor(long sequence, const Sequence& cursor,
			const SequenceGroup& dependent,
			const AlertableBarrier& barrier) {

		long availableSequence;

		if ((availableSequence = cursor.get()) < sequence) {
			std::unique_lock<std::mutex> ulock(m_mutex);

			while ((availableSequence = cursor.get()) < sequence) {
				barrier.checkAlert();
				std::cv_status status =
						m_condition.wait_for(ulock, std::chrono::microseconds(m_timeout_micros));
				if (status == std::cv_status::timeout) {
					break;
				}

			}

		}

		if (!dependent.isEmpty()) {
			while ((availableSequence = dependent.get()) < sequence) {
				barrier.checkAlert();
			}
		}

		return availableSequence;
	}

	void signalAllWhenBlocking() {
		m_condition.notify_all();
	}

	void configure(int64_t micros) {
		m_timeout_micros = micros;
	}

private:

	int64_t m_timeout_micros = 1000;
	std::mutex m_mutex;
	std::condition_variable m_condition;
};

INTERNAL_NAMESPACE_END

#endif /* TIMEOUT_BLOCKING_WAIT_STRATEGY_HPP_ */
