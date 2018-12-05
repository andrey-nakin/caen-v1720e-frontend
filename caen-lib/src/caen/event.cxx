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

void Event::decode(char* const eventPtr) {

	handle.hCommand("decoding event",
			[this, eventPtr](int handle) {return CAEN_DGTZ_DecodeEvent(handle, eventPtr, &event);});

}

}

