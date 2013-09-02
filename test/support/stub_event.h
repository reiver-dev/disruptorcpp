#ifndef DISRUPTOR_TEST_STUB_EVENT_H_
#define DISRUPTOR_TEST_STUB_EVENT_H_

#include <string>
#include "long_event.h"
#include <disruptor/disruptor.hpp>

using disruptor::EventHandler;

class StubEvent : public LongEvent {
 public:
    StubEvent(const int64_t& value = 0) : LongEvent(value) {}

    std::string test_string() const { return test_string_; }

    void set_test_string(const std::string& test_string) {
        test_string_ = test_string;
    }

 private:
    std::string test_string_;
};

class StubEventFactory {
 public:

    StubEvent* operator()(int size) const {
        return new StubEvent[size];
    }

	StubEvent* operator()() const {
		return new StubEvent();
	}

};

class StubBatchHandler : public EventHandler<StubEvent> {
 public:
    virtual void onEvent(long sequence,
                         bool end_of_batch,
                         StubEvent* event) {
        if (event)
            event->set_value(sequence);
    };

    virtual void onStart() {}
    virtual void onShutdown() {}
};

class StubEventTranslator {
public:
	void operator()(StubEvent* event, long sequence) {
		event->set_value(sequence);
	}
};

#endif // DISRUPTOR_TEST_LONG_EVENT_H_ NOLINT
