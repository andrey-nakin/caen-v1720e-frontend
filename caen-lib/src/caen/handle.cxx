#include <CAENDigitizer.h>
#include <caen/handle.hxx>

namespace caen {

Handle::Handle(int const linkNum, int const conetNode, ::uint32_t const vmeBaseAddr) : handle(0) {

	errorCode = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, linkNum, conetNode, vmeBaseAddr, &handle);

}

Handle::Handle(Handle&& h) : handle(0) {

	this->handle = h.handle;
	h.handle = 0;

}

Handle::~Handle() {

	if (handle) {
		CAEN_DGTZ_CloseDigitizer(handle);
	}

}

}

