#ifndef DISRUPTOR_UTILS_H_
#define DISRUPTOR_UTILS_H_

#include <atomic>
#include <bitset>

namespace Util {

int log2(int i) {
	int r = 0;
	while ((i >>= 1) != 0) {
		++r;
	}
	return r;
}

template<typename T>
constexpr bool isPow2(T number) {
	static_assert(std::is_integral<T>::value, "Available only for integer types");
	return std::bitset<sizeof(T)>(number).count() < 2;
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
