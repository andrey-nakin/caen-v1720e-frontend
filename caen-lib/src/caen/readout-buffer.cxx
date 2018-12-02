#include <CAENDigitizer.h>
#include <caen/readout-buffer.hxx>

namespace caen {

ReadoutBuffer::ReadoutBuffer(Handle& handle) : buffer(nullptr), bufferSize(0) {

	errorCode = CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &bufferSize);
	if (CAEN_DGTZ_Success != errorCode) {
		throw Exception(errorCode, "allocating readout buffer");
	}

}

ReadoutBuffer::ReadoutBuffer(ReadoutBuffer&& rb) : buffer(nullptr), bufferSize(0) {

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

}

