#include <utility>
#include "caen/device.hxx"

namespace caen {

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
		std::lock_guard < std::mutex > lock(mutex);
		buffer = std::unique_ptr < ReadoutBuffer > (new ReadoutBuffer(handle));
	}
	return *buffer;

}

Event& Device::getEvent() {

	if (!event) {
		std::lock_guard < std::mutex > lock(mutex);
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

}

void Device::stopAcquisition() {

	handle.hCommand("stopping acquisition", CAEN_DGTZ_SWStopAcquisition);

	// release buffers
	std::lock_guard < std::mutex > lock(mutex);
	event = nullptr;
	buffer = nullptr;

	reset();

}

bool Device::hasNextEvent() {

	if (eventNo >= numEvents) {
		dataSize = getBuffer().readData();
		if (dataSize) {
			numEvents = getBuffer().getNumEvents(dataSize);
			eventNo = 0;
		}
	}
	return eventNo < numEvents;

}

CAEN_DGTZ_UINT16_EVENT_t const* Device::nextEvent(
		CAEN_DGTZ_EventInfo_t& eventInfo) {

	if (!hasNextEvent()) {
		return nullptr;
	}

	std::pair<CAEN_DGTZ_EventInfo_t, char*> evt = getBuffer().getEventInfo(
			dataSize, eventNo);
	if (++eventNo >= numEvents) {
		reset();
	}

	eventInfo = evt.first;
	getEvent().decode(evt.second);

	return getEvent().evt();

}

void Device::reset() {

	dataSize = 0;
	numEvents = 0;
	eventNo = 0;

}

}
