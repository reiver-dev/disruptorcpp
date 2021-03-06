#ifndef BLOCKING_WAIT_STRATEGY_HPP_
#define BLOCKING_WAIT_STRATEGY_HPP_

#include <mutex>
#include <condition_variable>

#include "macro.hpp"
#include "sequence_barrier.hpp"
#include "sequence_group.hpp"

INTERNAL_NAMESPACE_BEGIN

class BlockingWaitStrategy {
public:

	long waitFor(long sequence, const Sequence& cursor,
			const SequenceGroup& dependent,
			const AlertableBarrier& barrier) {
		long availableSequence;

		if ((availableSequence = cursor.get()) < sequence) {
			std::unique_lock<std::mutex> ulock(m_mutex);

			while ((availableSequence = cursor.get()) < sequence) {
				barrier.checkAlert();
				m_condition.wait(ulock);
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

private:
	std::mutex m_mutex;
	std::condition_variable m_condition;
};

INTERNAL_NAMESPACE_END

#endif /* BLOCKING_WAIT_STRATEGY_HPP_ */
