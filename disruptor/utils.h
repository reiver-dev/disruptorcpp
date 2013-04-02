#ifndef DISRUPTOR_UTILS_H_
#define DISRUPTOR_UTILS_H_

#include <vector>
#include <algorithm>
#include <limits>

#include "sequence.hpp"



namespace Util {

//   public static int ceilingNextPowerOfTwo(final int x)
//    {
//        return 1 << (32 - Integer.numberOfLeadingZeros(x - 1));
//    }

long getMinimumSequence(const std::vector<Sequence*>& sequences, long minimum) {
	size_t n = sequences.size();
	for (size_t i = 0; i < n; i++) {
		Sequence *seq = sequences[i];
		long value = seq->get();
		minimum = std::min(minimum, value);
	}
	return minimum;
}

long getMinimumSequence(const std::vector<Sequence*>& sequences) {
	return getMinimumSequence(sequences, std::numeric_limits<long>::max());
}


//
//    /**
//     * Get an array of {@link Sequence}s for the passed {@link EventProcessor}s
//     *
//     * @param processors for which to get the sequences
//     * @return the array of {@link Sequence}s
//     */
//    public static Sequence[] getSequencesFor(final EventProcessor... processors)
//    {
//        Sequence[] sequences = new Sequence[processors.length];
//        for (int i = 0; i < sequences.length; i++)
//        {
//            sequences[i] = processors[i].getSequence();
//        }
//
//        return sequences;
//    }

int log2(int i) {
	int r = 0;
	while ((i >>= 1) != 0) {
		++r;
	}
	return r;
}

}

#endif // DISRUPTOR_UTILS_H_
