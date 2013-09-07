#ifndef SEQUENCE_GROUP_RC_HPP_
#define SEQUENCE_GROUP_RC_HPP_

#include <cassert>
#include <cstdlib>
#include <algorithm>

#include "sequence.hpp"
#include "object_storage.hpp"


INTERNAL_NAMESPACE_BEGIN

class RefCountedSequenceGroup : public Storable {
public:

	static RefCountedSequenceGroup* create(size_t length) {
		assert(length > 0);
		RefCountedSequenceGroup *g = (RefCountedSequenceGroup*)
				malloc(sizeof(RefCountedSequenceGroup) + (length - 1) * sizeof(uintptr_t));
		g->length = length;
		return g;
	}

	static RefCountedSequenceGroup* createEmpty() {
		RefCountedSequenceGroup *g = (RefCountedSequenceGroup*)
				malloc(sizeof(RefCountedSequenceGroup));
		g->length = 0;
		return g;
	}

	long getMinimumSequence(long minimum) const {
		for (size_t i = 0, n = length; i != n; i++) {
			long value = sequences[i]->get();
			minimum = std::min(minimum, value);
		}
		return minimum;
	}

	bool isEmpty() const {
		return length == 0;
	}

	size_t size() const {
		return length;
	}

	Sequence** begin() {
		return sequences;
	}

	Sequence** end() {
		return sequences + length;
	}

private:
	size_t length = 0;
	Sequence *sequences[1];

	DISALLOW_COPY_MOVE(RefCountedSequenceGroup);

};

struct DeleteSequenceGroup {
	void operator()(void *obj) const {
		free(obj);
	}
};

class SequenceGroupStorage : public ObjectStorage<4, DeleteSequenceGroup> {

	typedef ObjectStorage<4, DeleteSequenceGroup> BaseType;

public:
	SequenceGroupStorage() : BaseType(RefCountedSequenceGroup::createEmpty()) {
		//
	}

	template<typename Collection>
	void add(Collection&& sequences) {
		RefCountedSequenceGroup *oldSeq = (RefCountedSequenceGroup*) write_lock();

		size_t oldsize = oldSeq->size();
		RefCountedSequenceGroup *newSeq = RefCountedSequenceGroup::create(oldsize + sequences.size());

		std::copy(oldSeq->begin(), oldSeq->end(), newSeq->begin());
		std::copy(begin(sequences), end(sequences), newSeq->begin() + oldsize);

		write_unlock(newSeq);
	}

	template<typename Collection>
	void remove(Collection&& sequences) {
		RefCountedSequenceGroup *oldSeq = (RefCountedSequenceGroup*)write_lock();

		int numToRemove = std::count_if(oldSeq->begin(), oldSeq->end(), [&sequences](Sequence *s){
			return std::find(sequences.begin(), sequences.end(), s) != sequences.end();
		});

		RefCountedSequenceGroup *newSeq = RefCountedSequenceGroup::create(oldSeq->size() - numToRemove);

		std::copy_if(oldSeq->begin(), oldSeq->end(), newSeq->begin(), [sequences](Sequence *s){
			return std::find(sequences.begin(), sequences.end(), s) != sequences.end();
		});

		write_unlock(newSeq);
	}


private:

	DISALLOW_COPY_MOVE(SequenceGroupStorage);

};

//typedef ObjectStorage<4, DeleteSequenceGroup> SequenceGroupStorage;

INTERNAL_NAMESPACE_END

#endif /* RC_SEQUENCE_GROUP_HPP_ */
