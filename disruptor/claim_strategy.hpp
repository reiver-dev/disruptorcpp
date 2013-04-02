#ifndef CLAIM_STRATEGY_HPP_
#define CLAIM_STRATEGY_HPP_

#include "sequence.hpp"
#include "sequencer.hpp"
#include "publisher.hpp"

struct SingleThreadClaimStrategy {

	typedef SingleProducerPublisher Publisher;
	typedef SingleProducerSequencer Sequencer;

	Publisher publisher;
	Sequencer sequencer;

	SingleThreadClaimStrategy(int buffersize) : sequencer(buffersize) {

	}

	Sequence& getCursor() {
		return publisher.getCursorSequence();
	}
};


#endif /* CLAIM_STRATEGY_HPP_ */
