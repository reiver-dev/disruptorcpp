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
		long minimum = std::numeric_limits<long>::max();
		for (auto seq = sequences.begin(), end = sequences.end(); seq != end; seq++) {
			long value = (*seq)->get();
			minimum = std::min(minimum, value);
		}
		return minimum;
	}

	long getMinimumSequence(long minimum) const {
		for (auto seq = sequences.begin(), end = sequences.end(); seq != end; seq++) {
			long value = (*seq)->get();
			minimum = std::min(minimum, value);
		}
		return minimum;
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
