#include <iostream>
#include <midas.h>
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

	handle.hCommand("reading data",
			[this](int handle) {
				auto const status = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &dataSize);
				if (CAEN_DGTZ_Success != status) {
					cm_msg(MERROR, "v1720-test", "CAEN_DGTZ_ReadData dataSize=%u", dataSize);

					// dry read
					do {
						CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &dataSize);
					}while (dataSize > 0);

					dataSize = 0;
				}
				return status;
			});
	return dataSize;

}

uint32_t ReadoutBuffer::getNumEvents() {

	if (!dataSize) {
		return 0;
	}

	uint32_t numEvents;
	handle.hCommand("getting num events",
			[this, &numEvents](int handle) {return CAEN_DGTZ_GetNumEvents(handle, buffer, dataSize, &numEvents);});
	return numEvents;

}

char* ReadoutBuffer::getEventInfo(int32_t const numEvent,
		CAEN_DGTZ_EventInfo_t& eventInfo) {

	if (!dataSize) {
		return nullptr;
	}

	char* result;
	handle.hCommand("getting event info",
			[this, numEvent, &eventInfo, &result](int handle) {
				auto const status=CAEN_DGTZ_GetEventInfo(handle, buffer, dataSize, numEvent, &eventInfo, &result);
				if (CAEN_DGTZ_Success != status) {
					dataSize = 0;
				}
				return status;
			});
	return result;

}

}

