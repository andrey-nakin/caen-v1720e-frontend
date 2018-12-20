#ifndef	__CAEN_EVENT_HXX__
#define	__CAEN_EVENT_HXX__

#include <CAENDigitizer.h>
#include "noncopyable.hxx"
#include "error-holder.hxx"
#include "handle.hxx"

namespace caen {

class Event final : public Noncopyable, public ErrorHolder {

	Handle& handle;
	void *event;

public:

	Event(Handle&);
	Event(Event&&);
	~Event();

	CAEN_DGTZ_UINT16_EVENT_t const* evt(char* eventPtr);

private:

	void decode(char* eventPtr);

};

}

#endif	//	__CAEN_EVENT_HXX__

