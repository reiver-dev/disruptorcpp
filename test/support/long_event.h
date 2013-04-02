#ifndef DISRUPTOR_TEST_LONG_EVENT_H_
#define DISRUPTOR_TEST_LONG_EVENT_H_



class LongEvent {
public:
	LongEvent(int64_t value = 0) :
			value_(value) {
	}

	int64_t value() const {
		return value_;
	}

	void set_value(int64_t value) {
		value_ = value;
	}

private:
	int64_t value_;
};

class LongEventFactory {
public:
	LongEvent* NewInstance(const int& size) const {
		return new LongEvent[size];
	}

	LongEvent* operator()() {
		return new LongEvent();
	}
};

#endif // DISRUPTOR_TEST_LONG_EVENT_H_
