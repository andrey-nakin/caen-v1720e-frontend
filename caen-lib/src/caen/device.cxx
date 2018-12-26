#include <utility>
#include <midas.h>
#include "caen/device.hxx"

namespace caen {

static uint32_t maxNumOfEvents;

Device::Device(Handle&& aHandle) :
		handle(std::move(aHandle)) {

	reset();

}

Device::~Device() {

}

Handle& Device::getHandle() {

	return handle;

}

ReadoutBuffer& Device::getBuffer() {

	if (!buffer) {
		buffer = std::unique_ptr < ReadoutBuffer > (new ReadoutBuffer(handle));
	}
	return *buffer;

}

Event& Device::getEvent() {

	if (!event) {
		event = std::unique_ptr < Event > (new Event(handle));
	}
	return *event;

}

void Device::startAcquisition() {

	// create readout buffer if not yet created
	getBuffer();

	// create event buffer if not yet created
	getEvent();

	handle.hCommand("starting acquisition", CAEN_DGTZ_SWStartAcquisition);

	reset();

	maxNumOfEvents = 0;

}

void Device::stopAcquisition() {

	handle.hCommand("stopping acquisition", CAEN_DGTZ_SWStopAcquisition);

	// release buffers
	event = nullptr;
	buffer = nullptr;

	reset();

}

bool Device::hasNextEvent() {

	if (eventNo >= numEvents) {
		if (getBuffer().readData()) {
			numEvents = getBuffer().getNumEvents();
			eventNo = 0;

			if (maxNumOfEvents < numEvents) {
				cm_msg(MINFO, "v1720-test", "maxNumOfEvents = %u",
						maxNumOfEvents);
				maxNumOfEvents = numEvents;
			}
		}
	}
	return eventNo < numEvents;

}

CAEN_DGTZ_UINT16_EVENT_t const* Device::nextEvent(
		CAEN_DGTZ_EventInfo_t& eventInfo) {

	if (!hasNextEvent()) {
		return nullptr;
	}

	// read event to buffer
	char* dataPtr;
	try {
		dataPtr = getBuffer().getEventInfo(eventNo++, eventInfo);
	} catch (Exception& e) {
		reset();
		throw e;
	}

	// decode and return event data
	return getEvent().evt(dataPtr);

}

void Device::reset() {

	numEvents = eventNo = 0;

}

}
