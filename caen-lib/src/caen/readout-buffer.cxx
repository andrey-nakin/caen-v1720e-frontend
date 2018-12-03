#include <CAENDigitizer.h>
#include <caen/readout-buffer.hxx>

namespace caen {

ReadoutBuffer::ReadoutBuffer(Handle& aHandle) : handle(aHandle), buffer(nullptr), bufferSize(0) {

	errorCode = CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &bufferSize);
	if (CAEN_DGTZ_Success != errorCode) {
		throw Exception(errorCode, "allocating readout buffer");
	}

}

ReadoutBuffer::ReadoutBuffer(ReadoutBuffer&& rb) : handle(rb.handle), buffer(nullptr), bufferSize(0) {

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

std::pair<CAEN_DGTZ_EventInfo_t, char*> ReadoutBuffer::getEventInfo() {

	std::pair<CAEN_DGTZ_EventInfo_t, char*> result;
	errorCode = CAEN_DGTZ_GetEventInfo(handle, buffer, bufferSize, 0, &result.first, &result.second);
	if (CAEN_DGTZ_Success != errorCode) {
		throw Exception(errorCode, "getting event info");
	}
	return result;

}

uint32_t ReadoutBuffer::readData() {

	uint32_t dataSize;
	errorCode = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &dataSize);
	if (CAEN_DGTZ_Success != errorCode) {
		throw Exception(errorCode, "reading data");
	}
	return dataSize;

}

uint32_t ReadoutBuffer::getNumEvents(uint32_t const dataSize) {

	uint32_t numEvents;
	errorCode = CAEN_DGTZ_GetNumEvents(handle, buffer, dataSize, &numEvents);
	if (CAEN_DGTZ_Success != errorCode) {
		throw Exception(errorCode, "getting num events");
	}
	return numEvents;

}

}

