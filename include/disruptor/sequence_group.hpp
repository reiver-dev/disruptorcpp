#ifndef SEQUENCE_GROUP_HPP_
#define SEQUENCE_GROUP_HPP_

#include <algorithm>
#include "sequence.hpp"

INTERNAL_NAMESPACE_BEGIN

class SequenceGroup {
public:

	SequenceGroup() : sequences(0) {

	}

	template<typename Collection>
	SequenceGroup(Collection&& seq) : sequences(seq) {

	}

	template<typename Collection>
	void add(Collection&& collection) {
		sequences.assign(collection.begin(), collection.end());
	}

	template<typename Collection>
	void remove(Collection&& collection) {
		std::remove_if(sequences.begin(), sequences.end(),
				[&collection](Sequence *s) {
					return std::find(collection.begin(),
							collection.end(), s) != collection.end();
				});
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

private:
	std::vector<Sequence *> sequences;
};

INTERNAL_NAMESPACE_END

#endif /* SEQUENCE_GROUP_HPP_ */
