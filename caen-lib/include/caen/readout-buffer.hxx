#ifndef	__CAEN_READOUT_BUFFER_HXX__
#define	__CAEN_READOUT_BUFFER_HXX__

#include <cstdint>
#include <utility>
#include <CAENDigitizer.h>
#include "noncopyable.hxx"
#include "error-holder.hxx"
#include "handle.hxx"

namespace caen {

class ReadoutBuffer final : public Noncopyable, public ErrorHolder {

	Handle& handle;
	char* buffer;
	uint32_t bufferSize, dataSize;

public:

	ReadoutBuffer(Handle&);
	ReadoutBuffer(ReadoutBuffer&&);
	~ReadoutBuffer();

	uint32_t readData();

	uint32_t getNumEvents();

	char* getEventInfo(int32_t numEvent, CAEN_DGTZ_EventInfo_t& eventInfo);

};

}

#endif	//	__CAEN_READOUT_BUFFER_HXX__

