#ifndef	__CAEN_READOUT_BUFFER_HXX__
#define	__CAEN_READOUT_BUFFER_HXX__

#include <cstdint>
#include "noncopyable.hxx"
#include "error-holder.hxx"
#include "handle.hxx"

namespace caen {

class ReadoutBuffer final : public Noncopyable, public ErrorHolder {

	char* buffer;
	uint32_t bufferSize;

public:

	ReadoutBuffer(Handle&);
	ReadoutBuffer(ReadoutBuffer&&);
	~ReadoutBuffer();

	operator char*() {

		return buffer;

	}

};

}

#endif	//	__CAEN_READOUT_BUFFER_HXX__

