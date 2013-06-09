#ifndef ABSTRACT_SEQUENCER_HPP_
#define ABSTRACT_SEQUENCER_HPP_

#include <assert.h>
#include "macro.hpp"
#include "utils.hpp"

class AbstractSequencer {
public:

	AbstractSequencer(int buffersize) : bufferSize(buffersize) {
		assert(Util::isPow2(buffersize));
	}

	int getBufferSize() {
		return bufferSize;
	}

	Sequence& getCursorSequence() {
		return m_cursor;
	}

	template<typename Collection>
	void addGatingSequences(Collection&& sequences) {
		gatingSequences.assign(std::forward<Collection>(sequences));
	}

	long getMinimumSequence() {
		return gatingSequences.getMinimumSequence(m_cursor.get());
	}

	const SequenceGroup& getGatingSequences() const {
		return gatingSequences;
	}

protected:
	SequenceGroup gatingSequences;
	Sequence m_cursor;
	int bufferSize;

	DISALLOW_COPY_ASSIGN_MOVE(AbstractSequencer);
};



#endif /* ABSTRACT_SEQUENCER_HPP_ */
