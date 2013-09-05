#ifndef SEQUENCE_GROUP_RC_HPP_
#define SEQUENCE_GROUP_RC_HPP_

#include <cassert>
#include <cstdlib>

#include "sequence.hpp"
#include "object_storage.hpp"


INTERNAL_NAMESPACE_BEGIN

class SequenceGroupRc : public Storable {
public:

	static SequenceGroupRc* create(size_t length) {
		SequenceGroupRc *g = (SequenceGroupRc*)
				malloc(sizeof(SequenceGroupRc) + (length - 1) * sizeof(uintptr_t));
		g->length = length;
		return g;
	}

	static SequenceGroupRc* createEmpty() {
		SequenceGroupRc *g = (SequenceGroupRc*)
				malloc(sizeof(SequenceGroupRc));
		g->length = 0;
		return g;
	}

	Sequence*& at(size_t idx) {
		return sequences[idx];
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

private:
	size_t length = 0;
	Sequence *sequences[1];

	DISALLOW_COPY_MOVE(SequenceGroupRc);

};

struct DeleteSequenceGroup {
	void operator()(void *obj) const {
		free(obj);
	}
};

typedef ObjectStorage<4, DeleteSequenceGroup> SequenceGroupStorage;

INTERNAL_NAMESPACE_END

#endif /* RC_SEQUENCE_GROUP_HPP_ */
