#ifndef PUBLISHER_HPP_
#define PUBLISHER_HPP_

#include "wait_strategy.hpp"
#include "sequence.hpp"

/*
class Publisher {
public:
	void publish(long sequence);
	bool isAvalable(long sequence);
	void ensureAvalable(long sequence);
};
*/

class SingleProducerPublisher {
public:

    SingleProducerPublisher() {
        //
    }

    template<class WaitStrategy>
    void publish(long sequence, WaitStrategy& waitStrategy) {
        m_cursor.set(sequence);
        waitStrategy.signalAllWhenBlocking();
    }

    void ensureAvailable(long sequence) {
    	//
    }

    bool isAvailable(long sequence) const {
        return sequence <= m_cursor.get();
    }

    Sequence& getCursorSequence() {
        return m_cursor;
    }

private:
	Sequence m_cursor;
};


#endif /* PUBLISHER_HPP_ */
