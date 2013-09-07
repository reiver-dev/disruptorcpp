#ifndef ABSTRACT_SEQUENCER_HPP_
#define ABSTRACT_SEQUENCER_HPP_

#include <assert.h>
#include "macro.hpp"
#include "utils.hpp"
#include "sequence_group_rc.hpp"

INTERNAL_NAMESPACE_BEGIN

class AbstractSequencer {
public:

	AbstractSequencer(int buffersize)
	: bufferSize(buffersize)
	, gatingSequences() {
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
		gatingSequences.add(std::forward<Collection>(sequences));
	}

	template<typename Collection>
	void removeGatingSequences(Collection&& sequences) {
		gatingSequences.remove(std::forward<Collection>(sequences));
	}

protected:

	long getMinimumSequence(long min) {
		RefCountedSequenceGroup *seq = (RefCountedSequenceGroup*) gatingSequences.aquire();
		long result = seq->getMinimumSequence(min);
		gatingSequences.release(seq);
		return result;
	}

	Sequence m_cursor;
	int bufferSize;

	SequenceGroupStorage gatingSequences;

	DISALLOW_COPY_MOVE(AbstractSequencer);
};

INTERNAL_NAMESPACE_END

#endif /* ABSTRACT_SEQUENCER_HPP_ */
