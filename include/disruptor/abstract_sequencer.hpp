#ifndef ABSTRACT_SEQUENCER_HPP_
#define ABSTRACT_SEQUENCER_HPP_

#include <assert.h>
#include "macro.hpp"
#include "utils.hpp"
#include "sequence_group_rc.hpp"

INTERNAL_NAMESPACE_BEGIN

class AbstractSequencer {
public:

	typedef SequenceGroupRc SequenceGroup;

	AbstractSequencer(int buffersize)
	: bufferSize(buffersize)
	, gatingSequenceStorage(SequenceGroup::createEmpty()) {
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
		SequenceGroup *oldSeq = (SequenceGroup*)gatingSequenceStorage.write_lock();
		SequenceGroup *newSeq = SequenceGroup::create(oldSeq->size() + sequences.size());
		size_t i = 0;
		for (size_t n = oldSeq->size(); i < n; ++i) {
			newSeq->at(i) = oldSeq->at(i);
		}
		for (Sequence *s : sequences) {
			newSeq->at(i) = s;
			i++;
		}
		gatingSequenceStorage.write_unlock(newSeq);
	}

	template<typename Collection>
	void removeGatingSequences(Collection&& sequences) {
		SequenceGroup *oldSeq = (SequenceGroup*)gatingSequenceStorage.write_lock();

		int numToRemove = 0;
		for (size_t i = 0, n = oldSeq->size(); i != n; ++i) {
			for (Sequence *s : sequences) {
				if (oldSeq->at(i) == s) {
					numToRemove++;
				}
			}
		}

		SequenceGroup *newSeq = SequenceGroup::create(oldSeq->size() - numToRemove);

		for (size_t i = 0, n = oldSeq->size(), j = 0; i != n; ++i) {
			bool toRemove = false;
			for (Sequence *s : sequences) {
				if (oldSeq->at(i) == s) {
					toRemove = true;
					break;
				}
			}
			if (!toRemove) {
				newSeq->at(j) = oldSeq->at(i);
				j++;
			}
		}

		gatingSequenceStorage.write_unlock(newSeq);
	}




protected:

	long getMinimumSequence(long min) {
		SequenceGroup *seq = (SequenceGroup*)gatingSequenceStorage.aquire();
		long result = seq->getMinimumSequence(min);
		gatingSequenceStorage.release(seq);
		return result;
	}

	Sequence m_cursor;
	int bufferSize;

	SequenceGroupStorage gatingSequenceStorage;

	DISALLOW_COPY_MOVE(AbstractSequencer);
};

INTERNAL_NAMESPACE_END

#endif /* ABSTRACT_SEQUENCER_HPP_ */
