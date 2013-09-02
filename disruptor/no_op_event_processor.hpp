#ifndef NO_OP_EVENT_PROCESSOR_HPP_
#define NO_OP_EVENT_PROCESSOR_HPP_

#include "ring_buffer.hpp"

INTERNAL_NAMESPACE_BEGIN

template<class T, typename Sequencer, typename WaitStrategy>
class NoOpEventProcessor {

	typedef RingBuffer<T, Sequencer, WaitStrategy> RingBuffer_t;

public:

	NoOpEventProcessor(RingBuffer_t* ring_buffer) :
		m_ring_buffer(ring_buffer) {
	}

	Sequence* getSequencePtr() {
		return m_ring_buffer->getCursorPtr();
	}

	void Halt() {
	}

	void Run() {
	}

private:

	RingBuffer_t* m_ring_buffer;
};

INTERNAL_NAMESPACE_END

#endif /* NO_OP_EVENT_PROCESSOR_HPP_ */
