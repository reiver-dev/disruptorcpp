#ifndef BATCH_EVENT_PROCESSOR_HPP_
#define BATCH_EVENT_PROCESSOR_HPP_

#include <atomic>

#include "ring_buffer.hpp"
#include "sequence.hpp"
#include "sequence_barrier.hpp"
#include "event_handler.hpp"

template<class RingBuffer>
class BatchEventProcessor {
public:

	typedef RingBuffer RingBuffer_t;
	typedef typename RingBuffer::SequenceBarrier_t SequenceBarrier_t;
	typedef EventHandler<typename RingBuffer::ValueType> EventHandler_t;

	BatchEventProcessor(RingBuffer_t *ringBuffer,
			SequenceBarrier_t *sequenceBarrier, EventHandler_t *eventHandler) :
			m_running(false),
			m_ringBuffer(ringBuffer),
			m_sequenceBarrier(sequenceBarrier),
			m_eventHandler(eventHandler) {

	}

	Sequence* getSequence() {
		return &m_sequence;
	}

	void halt() {
		m_running.store(false);
		m_sequenceBarrier->alert();
	}

	void run() {
		bool fls = false;
		if (!m_running.compare_exchange_strong(fls, true)) {
			throw std::runtime_error("Already running");
		}

		m_sequenceBarrier->clearAlert();

		typename RingBuffer::ValueType* event = nullptr;

		long nextSequence = m_sequence.get() + 1L;
		while (true) {
			try {
				long availableSequence = m_sequenceBarrier->waitFor(nextSequence);
				while (nextSequence <= availableSequence) {
					event = m_ringBuffer->get(nextSequence);
					m_eventHandler->onEvent(nextSequence, nextSequence == availableSequence, event);
					nextSequence++;
				}
				m_sequence.set(availableSequence);
			} catch (AlertException &e) {
				if (!m_running.load())
					break;
			}
		}
		m_running.store(false);
	}

	void notifyTimeout(long availableSequence) {
	}

	void operator()() {
		run();
	}

private:

	std::atomic<bool> m_running;

	Sequence m_sequence;

	RingBuffer_t * const m_ringBuffer;
	SequenceBarrier_t * const m_sequenceBarrier;
	EventHandler_t * const m_eventHandler;

};

#endif /* BATCH_EVENT_PROCESSOR_HPP_ */
