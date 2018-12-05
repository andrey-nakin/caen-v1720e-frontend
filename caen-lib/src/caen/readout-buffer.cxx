#include <caen/readout-buffer.hxx>

namespace caen {

ReadoutBuffer::ReadoutBuffer(Handle& aHandle) :
		handle(aHandle), buffer(nullptr), bufferSize(0) {

	handle.hCommand("allocating readout buffer",
			[this](int handle) {return CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &bufferSize);});

}

ReadoutBuffer::ReadoutBuffer(ReadoutBuffer&& rb) :
		handle(rb.handle), buffer(nullptr), bufferSize(0) {

	this->buffer = rb.buffer;
	this->bufferSize = rb.bufferSize;
	rb.buffer = nullptr;
	rb.bufferSize = 0;

}

ReadoutBuffer::~ReadoutBuffer() {

	if (buffer) {
		CAEN_DGTZ_FreeReadoutBuffer(&buffer);
	}

}

uint32_t ReadoutBuffer::readData() {

	uint32_t dataSize;
	handle.hCommand("reading data",
			[this, &dataSize](int handle) {return CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &dataSize);});
	return dataSize;

}

uint32_t ReadoutBuffer::getNumEvents(uint32_t const dataSize) {

	uint32_t numEvents;
	handle.hCommand("getting num events",
			[this, dataSize, &numEvents](int handle) {return CAEN_DGTZ_GetNumEvents(handle, buffer, dataSize, &numEvents);});
	return numEvents;

}

std::pair<CAEN_DGTZ_EventInfo_t, char*> ReadoutBuffer::getEventInfo(
		uint32_t const dataSize, int32_t const numEvent) {

	std::pair<CAEN_DGTZ_EventInfo_t, char*> result;
	handle.hCommand("getting event info",
			[this, dataSize, numEvent, &result](int handle) {return CAEN_DGTZ_GetEventInfo(handle, buffer, dataSize, numEvent, &result.first, &result.second);});
	return result;

}

}

