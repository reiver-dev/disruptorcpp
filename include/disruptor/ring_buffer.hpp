#ifndef RING_BUFFER_HPP_
#define RING_BUFFER_HPP_

#include <cassert>

#include "sequence.hpp"
#include "sequence_barrier.hpp"
#include "sequence_group.hpp"

#include "macro.hpp"

INTERNAL_NAMESPACE_BEGIN

template<typename Sequencer, typename WaitStrategy>
class RingBufferImpl {
public:

	typedef Sequencer Sequencer_t;
	typedef WaitStrategy WaitStrategy_t;
	typedef SequenceBarrier<Sequencer, WaitStrategy> SequenceBarrier_t;

	long next() {
		return sequencer.next();
	}

	long tryNext() {
		return sequencer.tryNext();
	}

	void resetTo(long sequence) {
		sequencer.claim(sequence);
		sequencer.publish(sequence);
	}

	bool isPublished(long sequence) {
		return sequencer.isAvailable(sequence);
	}

	template<typename Container>
	void addGatingSequences(Container&& container) {
		sequencer.addGatingSequences(std::forward<Container>(container));
	}

	long getMinimumGatingSequence() {
		return sequencer.getMinimumGatingSequence();
	}

	template<typename Collection>
	SequenceBarrier_t* newBarrier(Collection&& sequencesToTrack) {
		return new SequenceBarrier_t(sequencer.getCursorSequence(), sequencesToTrack);
	}

	long getCursor() const {
		return sequencer.getCursorSequence().get();
	}

	bool hasAvailableCapacity(int requiredCapacity) {
		return sequencer.hasAvailableCapacity(requiredCapacity);
	}

	void publish(long sequence) {
		sequencer.publish(sequence);
		waitStrategy.signalAllWhenBlocking();
	}

	long remainingCapacity() {
		return sequencer.remainingCapacity();
	}

	template<typename... ARGS>
	void configureWaitStrategy(ARGS&&... args) {
		waitStrategy.configure(std::forward<ARGS>(args)...);
	}

protected:

	void claim(long sequence) {
		sequencer.claim(sequence);
	}

	void ensureAvailable(long sequence) {
		sequencer.hasAvailableCapacity(sequence);
	}

	Sequence* getCursorPtr() {
		return &sequencer.getCursorSequence();
	}

	Sequencer& getSequencer() {
		return sequencer;
	}

	WaitStrategy& getWaitStrategy() {
		return waitStrategy;
	}

	RingBufferImpl(int buffersize) :
			sequencer(buffersize) {
		//
	}

private:

	WaitStrategy waitStrategy;
	Sequencer sequencer;

	friend SequenceBarrier_t;

};

template<class T, typename Sequencer, typename WaitStrategy>
class RingBuffer: public RingBufferImpl<Sequencer, WaitStrategy> {
public:

	typedef T ValueType;

	template<typename EventFactory>
	RingBuffer(int buffersize, EventFactory factory) :
			BaseType(buffersize), m_indexMask(buffersize - 1), m_bufferSize(
					buffersize), m_entries(new ValueType[buffersize]) {
		fill(buffersize, factory);
	}

	~RingBuffer() {
//		destroy(m_bufferSize);
		delete[] m_entries;
	}

	T* claimAndGetPreallocated(long sequence) {
		BaseType::claim(sequence);
		return get(sequence);
	}

	T* get(long sequence) {
		return &m_entries[(int) sequence & m_indexMask];
	}

	template<class EventTranslator>
	void publishEvent(EventTranslator translator) {
		long sequence = BaseType::next();
		translator(get(sequence), sequence);
		BaseType::publish(sequence);
	}

	template<class EventTranslator>
	bool tryPublishEvent(EventTranslator translator) {
		long sequence = BaseType::next();
		translator(get(sequence), sequence);
		BaseType::publish(sequence);
		return true;
	}

	int getBufferSize() const {
		return m_bufferSize;
	}

private:

	typedef RingBufferImpl<Sequencer, WaitStrategy> BaseType;

	template<typename A1, typename A2, typename A3>
	friend class NoOpEventProcessor;

	int m_indexMask;
	int m_bufferSize;
	ValueType* m_entries;

	template<typename EventFactory>
	void fill(int size, EventFactory factory) {
		for (int i = 0; i < size; i++) {
			factory(&m_entries[i]);
		}
	}

	void destroy(int size) {
		for (int i = 0; i < size; i++) {
			delete m_entries[i];
		}
	}

	DISALLOW_COPY_MOVE(RingBuffer);

};

INTERNAL_NAMESPACE_END

#endif /* RING_BUFFER_HPP_ */
