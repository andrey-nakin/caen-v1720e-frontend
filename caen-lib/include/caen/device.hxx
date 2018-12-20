#ifndef	__CAEN_DEVICE_HXX__
#define	__CAEN_DEVICE_HXX__

#include <memory>
#include <mutex>
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

private:

	std::mutex mutex;
	Handle handle;
	std::unique_ptr<ReadoutBuffer> buffer;
	std::unique_ptr<Event> event;

};

}

#endif	//	__CAEN_DEVICE_HXX__

