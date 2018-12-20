#ifndef	__CAEN_DEVICE_HXX__
#define	__CAEN_DEVICE_HXX__

#include <memory>
#include "noncopyable.hxx"
#include "handle.hxx"
#include "readout-buffer.hxx"
#include "event.hxx"

namespace caen {

class Device final : public Noncopyable {
public:

	Device(Handle&&);
	~Device();

	Handle& getHandle();

	ReadoutBuffer& getBuffer();

	Event& getEvent();

	void startAcquisition();

	void stopAcquisition();

	bool hasNextEvent();

	CAEN_DGTZ_UINT16_EVENT_t const* nextEvent(CAEN_DGTZ_EventInfo_t& eventInfo);

private:

	Handle handle;
	std::unique_ptr<ReadoutBuffer> buffer;
	std::unique_ptr<Event> event;
	uint32_t dataSize, numEvents, eventNo;

	void reset();

};

}

#endif	//	__CAEN_DEVICE_HXX__

