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
		return Util::getMinimumSequence(sequences);
	}

	bool isEmpty() const {
		return sequences.size() == 0;
	}

	const std::vector<Sequence *>& data() const {
		return sequences;
	}

private:
	std::vector<Sequence *> sequences;
};

#endif /* SEQUENCE_GROUP_HPP_ */
