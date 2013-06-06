#ifndef SEQUENCE_BARRIER_HPP_
#define SEQUENCE_BARRIER_HPP_

#include "macro.hpp"
#include "sequence.hpp"
#include "sequence_group.hpp"


class AlertException : std::exception {

};

class AlertableBarrier {
public:

	AlertableBarrier(bool value) : m_alerted(value) {
		//
	}

	bool isAlerted() const {
		return m_alerted.load(std::memory_order::memory_order_acquire);
	}

	void checkAlert() const {
		if (isAlerted())
			throw AlertException();
	}

	void clearAlert() {
		m_alerted.store(false, std::memory_order::memory_order_release);
	}

protected:
	void setAlert() {
		m_alerted.store(true, std::memory_order::memory_order_release);
	}


private:
	std::atomic<bool> m_alerted;
};


template <typename WaitStrategy>
class SequenceBarrier : public AlertableBarrier {
public:

	SequenceBarrier() : AlertableBarrier(false), m_cursor(), dependentSequence(), waitStrategy(nullptr) {

	}

	template<typename RB>
	SequenceBarrier(RB& ringBuffer) :
		AlertableBarrier(false), m_cursor(ringBuffer.getSequence()), waitStrategy(ringBuffer.getWaitStrategy()) {

	}

	template<typename RB, typename Collection>
	SequenceBarrier(RB& ringBuffer, Collection&& dep_sequences) :
		AlertableBarrier(false), m_cursor(ringBuffer.getSequence()), dependentSequence(dep_sequences), waitStrategy(ringBuffer.getWaitStrategy()) {

	}

	long waitFor(long sequence) {
		return waitStrategy.waitFor(sequence, m_cursor, dependentSequence, *this);
	}

	long waitFor(long sequence, long micros) {
		return waitStrategy.waitFor(sequence, m_cursor, dependentSequence, *this, micros);
	}

	long getCursor() const {
		return m_cursor.get();
	}

	void alert() {
		setAlert();
		waitStrategy.signalAllWhenBlocking();
	}

private:

	const Sequence& m_cursor;
	const SequenceGroup dependentSequence;

	WaitStrategy& waitStrategy;

	DISALLOW_COPY_ASSIGN_MOVE(SequenceBarrier);
};

#endif /* SEQUENCE_BARRIER_HPP_ */
