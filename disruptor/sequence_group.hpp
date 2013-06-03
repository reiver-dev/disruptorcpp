#ifndef SEQUENCE_GROUP_HPP_
#define SEQUENCE_GROUP_HPP_

#include "utils.h"

class SequenceGroup {
public:

	SequenceGroup() : sequences(0) {

	}

	template<typename Collection>
	SequenceGroup(Collection&& seq) : sequences(seq) {

	}

	template<typename Collection>
	void assign(Collection&& collection) {
		sequences.assign(collection.begin(), collection.end());
	}

	long get() const {
		return Util::getMinimumSequence(begin(), end());
	}

	bool isEmpty() const {
		return sequences.size() == 0;
	}

	std::vector<Sequence *>::const_iterator begin() const {
		return sequences.begin();
	}

	std::vector<Sequence *>::const_iterator end() const {
		return sequences.end();
	}

private:
	std::vector<Sequence *> sequences;
};

#endif /* SEQUENCE_GROUP_HPP_ */
