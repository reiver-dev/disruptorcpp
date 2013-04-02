#ifndef NO_OP_EVENT_PROCESSOR_HPP_
#define NO_OP_EVENT_PROCESSOR_HPP_

#include "ring_buffer.hpp"

template<class T, typename ClaimStrategy, typename WaitStrategy>
class NoOpEventProcessor {

	typedef RingBuffer<T, ClaimStrategy, WaitStrategy> RingBuffer_t;

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


#endif /* NO_OP_EVENT_PROCESSOR_HPP_ */
