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

template<typename Iterator>
long getMinimumSequence(Iterator begin, Iterator end, long minimum) {
	for (Iterator seq = begin; seq != end; seq++) {
		long value = (*seq)->get();
		minimum = std::min(minimum, value);
	}
	return minimum;
}

template<typename Iterator>
long getMinimumSequence(Iterator begin, Iterator end) {
	return getMinimumSequence(begin, end, std::numeric_limits<long>::max());
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

template <typename T>
void arraySetAtomic(T *array, int shift, T value) {
	static_assert(sizeof(std::atomic<T>) == sizeof(T), "Cannot atomically write array index");
	return reinterpret_cast<std::atomic<int>*>(&array[shift])->store(value, std::memory_order_release);
}

template <typename T>
T arrayGetAtomic(T *array, int shift) {
	static_assert(sizeof(std::atomic<T>) == sizeof(T), "Cannot atomically read array index");
	return reinterpret_cast<std::atomic<int>*>(&array[shift])->load(std::memory_order_acquire);
}

}

#endif // DISRUPTOR_UTILS_H_
