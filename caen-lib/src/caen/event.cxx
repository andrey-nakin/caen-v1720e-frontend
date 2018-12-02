#include <caen/event.hxx>

namespace caen {

Event::Event(Handle& aHandle) : handle(aHandle), event(nullptr) {

	errorCode = CAEN_DGTZ_AllocateEvent(handle, &event);

}

Event::Event(Event&& rb) : handle(rb.handle), event(nullptr) {

	this->event = rb.event;
	rb.event = nullptr;

}

Event::~Event() {

	if (event) {
		CAEN_DGTZ_FreeEvent(handle, &event);
	}

}

void Event::decode(char* const eventPtr) {

	errorCode = CAEN_DGTZ_DecodeEvent(handle, eventPtr, &event);

}

}

