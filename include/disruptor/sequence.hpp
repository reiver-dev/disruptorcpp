#ifndef SEQUENCE_HPP_
#define SEQUENCE_HPP_

#include <atomic>

#include "macro.hpp"

INTERNAL_NAMESPACE_BEGIN

class FlatSequence {
public:

	static const long INITIAL_VALUE = -1;

	FlatSequence() : m_value(INITIAL_VALUE) {
		//
	}

	FlatSequence(long initial_value) : m_value(initial_value) {
		//
	}

	long get() const {
		return m_value.load(std::memory_order::memory_order_acquire);
	}

	void set(long value) {
		m_value.store(value, std::memory_order::memory_order_release);
	}

	long IncrementAndGet(const int64_t& increment) {
		long old = m_value.fetch_add(increment, std::memory_order::memory_order_release);
		return old + increment;
	}

	bool compareAndSet(long expected, long value) {
		m_value.compare_exchange_strong(expected, value);
		return false;
	}

private:
	std::atomic<long> m_value;

	DISALLOW_COPY_MOVE(FlatSequence);
};

//----------------------------------------------------

struct LeftPadding {
	char pad[CACHE_LINE_SIZE_IN_BYTES / 2];
};

struct RightPadding {
	char pad[ATOMIC_SEQUENCE_PADDING_LENGTH / 2];
};


class DoublePaddedSequence : private LeftPadding, public FlatSequence, private RightPadding {
public:
	DoublePaddedSequence() : FlatSequence() {
		//
	}

	DoublePaddedSequence(long initial_value) : FlatSequence(initial_value) {
		//
	}

	DISALLOW_COPY_MOVE(DoublePaddedSequence);
};

class PaddedSequence : public FlatSequence {
public:
	PaddedSequence() : FlatSequence() {
		//
	}

	PaddedSequence(long initial_value) : FlatSequence(initial_value) {
		//
	}

	char pad[ATOMIC_SEQUENCE_PADDING_LENGTH];

	DISALLOW_COPY_MOVE(PaddedSequence);
};

using Sequence = DoublePaddedSequence;

INTERNAL_NAMESPACE_END

#endif /* SEQUENCE_HPP_ */
