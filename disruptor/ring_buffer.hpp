#ifndef RING_BUFFER_HPP_
#define RING_BUFFER_HPP_

#include <assert.h>

#include "sequence.hpp"
#include "sequence_group.hpp"
#include "wait_strategy.hpp"

#include "claim_strategy.hpp"


template<typename ClaimStrategy, typename WaitStrategy>
class RingBufferImpl {
public:

	typedef ClaimStrategy ClaimStrategy_t;
	typedef WaitStrategy  WaitStrategy_t;

    long next() {
		return sequencer.next(gatingSequences);
	}

    long tryNext() {
        return sequencer.tryNext(gatingSequences);
    }

    void resetTo(long sequence)
    {
        sequencer.claim(sequence);
        publish(sequence);
    }

    bool isPublished(long sequence)
    {
        return publisher.isAvailable(sequence);
    }

    template<typename Container>
    void addGatingSequences(Container&& container) {
    	gatingSequences.assign(std::forward<Container>(container));
    }

    long getMinimumGatingSequence() {
		return Util::getMinimumSequence(gatingSequences.data(), cursor.get());
	}

    template<typename Collection>
	SequenceBarrier* newBarrier(Collection&& sequencesToTrack) {
		return new SequenceBarrier(cursor, sequencesToTrack);
	}

	long getCursor() const {
		return cursor.get();
	}

	const Sequence& getSequence() const {
		return cursor;
	}

	bool hasAvailableCapacity(int requiredCapacity) {
		return sequencer.hasAvailableCapacity(gatingSequences, requiredCapacity);
	}

	void publish(long sequence) {
		publisher.publish(sequence, waitStrategy);
	}

	long remainingCapacity() {
		return sequencer.remainingCapacity(gatingSequences);
	}

	WaitStrategy& getWaitStrategy() {
		return waitStrategy;
	}

protected:

	void claim(long sequence) {
		sequencer.claim(sequence);
	}

	void ensureAvailable(long sequence) {
        publisher.ensureAvailable(sequence);
	}


	Sequence* getCursorPtr() {
		return &cursor;
	}

	RingBufferImpl(int buffersize) :
			claimStrategy(buffersize),
			publisher(claimStrategy.publisher),
			sequencer(claimStrategy.sequencer),
			cursor(claimStrategy.getCursor())
	{
		//
	}

private:

    WaitStrategy waitStrategy;
    ClaimStrategy claimStrategy;

    typename ClaimStrategy::Publisher& publisher;
    typename ClaimStrategy::Sequencer& sequencer;

    Sequence& cursor;
    SequenceGroup gatingSequences;
};


template<class T, typename ClaimStrategy, typename WaitStrategy>
class RingBuffer : public RingBufferImpl<ClaimStrategy, WaitStrategy> {
private:
	typedef RingBufferImpl<ClaimStrategy, WaitStrategy> BaseType;

	template<typename A1, typename A2, typename A3>
	friend class NoOpEventProcessor;

    int m_indexMask;
    int m_bufferSize;
    T** m_entries;

	template<typename EventFactory>
	void fill(int size, EventFactory factory) {
		for (int i = 0; i < size; i++) {
			m_entries[i] = factory();
		}
	}

	void destroy(int size) {
		for (int i = 0; i < size; i++) {
			delete m_entries[i];
		}
	}

public:

	typedef T ValueType;

	template<typename EventFactory>
	RingBuffer(int buffersize, EventFactory factory) :
			BaseType(buffersize),
			m_indexMask(buffersize - 1),
			m_bufferSize(buffersize),
			m_entries(new T*[buffersize]) {
		fill(buffersize, factory);
	}

	~RingBuffer() {
		destroy(m_bufferSize);
		delete[] m_entries;
	}

	T* claimAndGetPreallocated(long sequence) {
		BaseType::claim(sequence);
		return getPreallocated(sequence);
	}

    T* getPublished(long sequence) {
    	BaseType::ensureAvailable(sequence);
		return m_entries[(int) sequence & m_indexMask];
	}

    T* getPreallocated(long sequence) {
		return m_entries[(int) sequence & m_indexMask];
	}

	template<class EventTranslator>
	void publishEvent(EventTranslator translator) {
		long sequence = BaseType::next();
		translator(getPreallocated(sequence), sequence);
		BaseType::publish(sequence);
	}

	template<class EventTranslator>
	bool tryPublishEvent(EventTranslator translator) {
		long sequence = BaseType::next();
		translator(getPreallocated(sequence), sequence);
		BaseType::publish(sequence);
		return true;
	}

	int getBufferSize() const {
		return m_bufferSize;
	}

};



#endif /* RING_BUFFER_HPP_ */
