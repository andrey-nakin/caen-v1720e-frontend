#ifndef	__CAEN_READOUT_BUFFER_HXX__
#define	__CAEN_READOUT_BUFFER_HXX__

#include <cstdint>
#include "noncopyable.hxx"
#include "error-holder.hxx"
#include "handle.hxx"

namespace caen {

class ReadoutBuffer final : public Noncopyable, public ErrorHolder {

	Handle& handle;
	char* buffer;
	uint32_t bufferSize;

public:

	ReadoutBuffer(Handle&);
	ReadoutBuffer(ReadoutBuffer&&);
	~ReadoutBuffer();

	uint32_t size() const {

		return bufferSize;

	}

	operator char*() {

		return buffer;

	}

	void getEventInfo(CAEN_DGTZ_EventInfo_t &eventInfo, char*&eventPtr);

	uint32_t readData();

	uint32_t getNumEvents(uint32_t dataSize);

};

}

#endif	//	__CAEN_READOUT_BUFFER_HXX__

