#include <caen/event.hxx>

namespace caen {

Event::Event(Handle& aHandle) :
		handle(aHandle), event(nullptr) {

	handle.hCommand("allocating event",
			[this](int handle) {return CAEN_DGTZ_AllocateEvent(handle, &event);});

}

Event::Event(Event&& rb) :
		handle(rb.handle), event(nullptr) {

	this->event = rb.event;
	rb.event = nullptr;

}

Event::~Event() {

	if (event) {
		CAEN_DGTZ_FreeEvent(handle, &event);
	}

}

CAEN_DGTZ_UINT16_EVENT_t const* Event::evt(char* eventPtr) {

	decode(eventPtr);
	return static_cast<CAEN_DGTZ_UINT16_EVENT_t*>(event);

}

void Event::decode(char* const eventPtr) {

	handle.hCommand("decoding event",
			[this, eventPtr](int handle) {return CAEN_DGTZ_DecodeEvent(handle, eventPtr, &event);});

}

}

