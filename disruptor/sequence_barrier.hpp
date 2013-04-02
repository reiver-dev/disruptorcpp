#ifndef SEQUENCE_BARRIER_HPP_
#define SEQUENCE_BARRIER_HPP_

#include "macro.hpp"
#include "sequence.hpp"
#include "wait_strategy.hpp"


class AlertException : std::exception {

};

class SequenceBarrier {
public:

	template<typename RB>
	SequenceBarrier(const RB& ringBuffer) :
		m_alerted(false), m_cursor(ringBuffer.getSequence()) {
	}

	template<typename RB, typename Collection>
	SequenceBarrier(const RB& ringBuffer, Collection&& dep_sequences) :
		m_alerted(false), m_cursor(ringBuffer.getSequence()) {
	}

	template<typename Collection>
	SequenceBarrier(const Sequence& sequence, Collection&& dep_sequences) :
		m_alerted(false), m_cursor(sequence), dependentSequence(dep_sequences) {
	}

	template<class WaitStrategy>
	long waitFor(long sequence, WaitStrategy& wait_strategy) {
		return wait_strategy.waitFor(sequence, m_cursor, dependentSequence, *this);
	}

	template<class WaitStrategy>
	long waitFor(long sequence, long micros, WaitStrategy& wait_strategy) {
		return wait_strategy.waitFor(sequence, m_cursor, dependentSequence, *this, micros);
	}

	long getCursor() const {
		return m_cursor.get();
	}

	bool isAlerted() const {
		return m_alerted.load(std::memory_order::memory_order_acquire);
	}

	template<class WaitStrategy>
	void alert(WaitStrategy& wait_strategy) {
		m_alerted.store(true, std::memory_order::memory_order_release);
		wait_strategy.signalAllWhenBlocking();
	}

	void clearAlert() {
		m_alerted.store(false, std::memory_order::memory_order_release);
	}

	void checkAlert() const {
		if (isAlerted())
			throw AlertException();
	}

private:
	std::atomic<bool> m_alerted;

	const Sequence& m_cursor;
	SequenceGroup dependentSequence;

	DISALLOW_COPY_ASSIGN_MOVE(SequenceBarrier);
};

#endif /* SEQUENCE_BARRIER_HPP_ */
