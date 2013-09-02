#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE RingBufferTest

#include <exception>
#include <future>
#include <thread>
#include <vector>
#include <array>

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <disruptor/disruptor.hpp>

#include "support/stub_event.h"

#define BUFFER_SIZE 64
#define LOG(x) std::cout << x << std::endl
#define LOGV(x) std::cout << #x " = " << x << std::endl


using namespace disruptor;


typedef RingBuffer<StubEvent, SingleProducerSequencer, BlockingWaitStrategy> RingBuffer_t;
typedef detail::NoOpEventProcessor<StubEvent, SingleProducerSequencer, BlockingWaitStrategy> EventProcessor_t;
typedef RingBuffer_t::SequenceBarrier_t SequenceBarrier_t;

using disruptor::detail::Sequence;

namespace test {

struct RingBufferFixture {

	RingBufferFixture() :
			ring_buffer(BUFFER_SIZE, StubEventFactory()),
			stub_processor(&ring_buffer),
			barrier(ring_buffer) {

		std::array<Sequence*, 1> sequences = {{stub_processor.getSequencePtr()}};
		ring_buffer.addGatingSequences(sequences);
	}

    ~RingBufferFixture() {}

    void FillBuffer() {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            int64_t sequence = ring_buffer.next();
            ring_buffer.publish(sequence);
        }
    }

    RingBuffer_t ring_buffer;
    EventProcessor_t stub_processor;
    SequenceBarrier_t barrier;
};

std::vector<StubEvent> Waiter(RingBuffer_t* ring_buffer,
                              SequenceBarrier_t* barrier,
                              int64_t initial_sequence,
                              int64_t to_wait_for_sequence) {
    barrier->waitFor(to_wait_for_sequence);

    std::vector<StubEvent> results;
    for (int64_t i = initial_sequence; i <= to_wait_for_sequence; i++)
        results.push_back(*ring_buffer->get(i));

    return results;
}


template<class T, typename WaitStrategy>
class TestEventProcessor {
 public:
    TestEventProcessor(SequenceBarrier_t* barrier) :
        barrier_(barrier) {}

    detail::Sequence* getSequence() { return &sequence_; }

    void halt() {}
    void run() {
        try {
            barrier_->waitFor(0L);
        } catch(...) {
            throw std::runtime_error("catched exception in TestEventProcessor::Run()");
        }

        sequence_.set(sequence_.get() + 1L);
    }

 private:

    Sequence sequence_;
    SequenceBarrier_t* barrier_;
};

BOOST_AUTO_TEST_SUITE(RingBufferBasic)

BOOST_FIXTURE_TEST_CASE(shouldClaimAndGet, RingBufferFixture) {
    BOOST_CHECK(ring_buffer.getCursor() == Sequence::INITIAL_VALUE);
    StubEvent expected_event(1234);

    long claim_sequence = ring_buffer.next();
    StubEvent* old_event = ring_buffer.get(claim_sequence);
    old_event->set_value(expected_event.value());
    ring_buffer.publish(claim_sequence);

    long sequence = barrier.waitFor(0, 5000);
    BOOST_CHECK(sequence == 0);

    StubEvent* event = ring_buffer.get(sequence);
    BOOST_CHECK(event->value() == expected_event.value());

    BOOST_CHECK(ring_buffer.getCursor() == 0);
}

BOOST_FIXTURE_TEST_CASE(ShouldClaimAndGetWithTimeout, RingBufferFixture) {
    BOOST_CHECK(ring_buffer.getCursor() == Sequence::INITIAL_VALUE);
    StubEvent expected_event(1234);

    int64_t claim_sequence = ring_buffer.next();
    StubEvent* old_event = ring_buffer.get(claim_sequence);
    old_event->set_value(expected_event.value());
    ring_buffer.publish(claim_sequence);

    int64_t sequence = barrier.waitFor(0, 5000);
    BOOST_CHECK(sequence == 0);

    StubEvent* event = ring_buffer.get(sequence);
    BOOST_CHECK(event->value() == expected_event.value());

    BOOST_CHECK(ring_buffer.getCursor() == 0);
}

BOOST_FIXTURE_TEST_CASE(ShouldGetWithTimeout, RingBufferFixture) {
    int64_t sequence = barrier.waitFor(0, 5000);
    BOOST_CHECK(sequence == Sequence::INITIAL_VALUE);
}

BOOST_FIXTURE_TEST_CASE(ShouldClaimAndGetInSeperateThread, RingBufferFixture) {
    std::future<std::vector<StubEvent>> future = \
        std::async(std::bind(&Waiter, &ring_buffer, &barrier, 0LL, 0LL));

    StubEvent expected_event(1234);

    int64_t sequence = ring_buffer.next();
    StubEvent* old_event = ring_buffer.get(sequence);
    old_event->set_value(expected_event.value());
    ring_buffer.publish(sequence);

    std::vector<StubEvent> results = future.get();

    BOOST_CHECK(results[0].value() == expected_event.value());
}

BOOST_FIXTURE_TEST_CASE(ShouldWrap, RingBufferFixture) {
    int n_messages = BUFFER_SIZE;
    int offset = 1000;

    for (int i = 0; i < n_messages + offset; i++) {
        int64_t sequence = ring_buffer.next();
        StubEvent* event = ring_buffer.get(sequence);
        event->set_value(i);
        ring_buffer.publish(sequence);
    }

    int expected_sequence = n_messages + offset - 1;
    int64_t avalaible= barrier.waitFor(expected_sequence);
    BOOST_CHECK(avalaible == expected_sequence);

    for (int i = offset; i < n_messages; i++) {
        BOOST_CHECK(i == ring_buffer.get(i)->value());
    }
}

BOOST_FIXTURE_TEST_CASE(ShouldGetAtSpecificSequence, RingBufferFixture) {
    int64_t expected_sequence = 5;

    StubEvent* expected_event = ring_buffer.claimAndGetPreallocated(expected_sequence);
    expected_event->set_value((int) expected_sequence);
    ring_buffer.publish(expected_sequence);

    int64_t sequence = barrier.waitFor(expected_sequence);
    BOOST_CHECK(expected_sequence == sequence);

    StubEvent* event = ring_buffer.get(sequence);
    BOOST_CHECK(expected_event->value() == event->value());

    BOOST_CHECK(expected_sequence == ring_buffer.getCursor());
}

// Publisher will try to publish BUFFER_SIZE + 1 events. The last event
// should wait for at least one consume before publishing, thus preventing
// an overwrite. After the single consume, the publisher should resume and
// publish the last event.
BOOST_FIXTURE_TEST_CASE(ShouldPreventPublishersOvertakingEventProcessorWrapPoint, RingBufferFixture) {
    std::atomic<bool> publisher_completed(false);
    std::atomic<int> counter(0);
    std::vector<Sequence*> dependency(0);
    SequenceBarrier_t sequenceBarrier(ring_buffer, dependency);
    TestEventProcessor<StubEvent, BlockingWaitStrategy> processor(&sequenceBarrier);
    dependency.push_back(processor.getSequence());
    ring_buffer.addGatingSequences(dependency);

    // Publisher in a seperate thread
    std::thread thread(
            // lambda definition
            [](RingBuffer_t* ring_buffer,
               SequenceBarrier_t* barrier,
               std::atomic<bool>* publisher_completed,
               std::atomic<int>* counter) {
            // body
                for (int i = 0; i <= BUFFER_SIZE; i++) {
                    int64_t sequence = ring_buffer->next();
                    StubEvent* event = ring_buffer->get(sequence);
                    event->set_value(i);
                    ring_buffer->publish(sequence);
                    counter->fetch_add(1L);
                }

                publisher_completed->store(true);
            }, // end of lambda
            &ring_buffer,
            &barrier,
            &publisher_completed,
            &counter);

    while (counter.load() < BUFFER_SIZE) {}

    int64_t sequence = ring_buffer.getCursor();
    BOOST_CHECK(sequence == (BUFFER_SIZE - 1));
    BOOST_CHECK(publisher_completed.load() == false);

    processor.run();
    thread.join();

    BOOST_CHECK(publisher_completed.load());
}

BOOST_AUTO_TEST_SUITE_END()

}
