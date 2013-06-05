#ifndef EVENT_HANDLER_HPP_
#define EVENT_HANDLER_HPP_

template<typename T>
class EventHandler {
public:
	virtual void onEvent(long sequence, bool end_of_batch,
			T* event) = 0;

	virtual void onStart() = 0;

	virtual void onShutdown() = 0;

	virtual ~EventHandler() {

	}

};


#endif /* EVENT_HANDLER_HPP_ */
