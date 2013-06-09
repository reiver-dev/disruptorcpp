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


template <typename Sequencer, typename WaitStrategy>
class SequenceBarrier : public AlertableBarrier {
public:

	template<typename RB>
	SequenceBarrier(RB& ringBuffer) :
		AlertableBarrier(false),
		sequencer(ringBuffer.getSequencer()),
		waitStrategy(ringBuffer.getWaitStrategy()),
		m_cursor(sequencer.getCursorSequence()) {

	}

	template<typename RB, typename Collection>
	SequenceBarrier(RB& ringBuffer, Collection&& dep_sequences) :
		AlertableBarrier(false),
		sequencer(ringBuffer.getSequencer()),
		waitStrategy(ringBuffer.getWaitStrategy()),
		m_cursor(sequencer.getCursorSequence()),
		dependentSequence(dep_sequences) {

	}

	long waitFor(long sequence) {
		checkAlert();
		long availableSequence = waitStrategy.waitFor(sequence, m_cursor, dependentSequence, *this);
		if (availableSequence < sequence) {
			return availableSequence;
		}
		return sequencer.getHighestPublishedSequence(sequence, availableSequence);
	}

	long waitFor(long sequence, long micros) {
		checkAlert();
		long availableSequence = waitStrategy.waitFor(sequence, m_cursor, dependentSequence, *this, micros);
		if (availableSequence < sequence) {
			return availableSequence;
		}
		return sequencer.getHighestPublishedSequence(sequence, availableSequence);
	}

	long getCursor() const {
		return m_cursor.get();
	}

	void alert() {
		setAlert();
		waitStrategy.signalAllWhenBlocking();
	}

private:

	Sequencer& sequencer;
	WaitStrategy& waitStrategy;

	const Sequence& m_cursor;
	const SequenceGroup dependentSequence;

	DISALLOW_COPY_ASSIGN_MOVE(SequenceBarrier);
};

#endif /* SEQUENCE_BARRIER_HPP_ */
