#include <sstream>
#include <CAENDigitizer.h>
#include <caen/handle.hxx>

namespace caen {

Handle::Handle(int const linkNum, int const conetNode, ::uint32_t const vmeBaseAddr) : handle(0) {

	errorCode = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, linkNum, conetNode, vmeBaseAddr, &handle);
	if (CAEN_DGTZ_Success != errorCode) {
		std::stringstream s;
		s << "opening device on link # " << linkNum << ", conet node # " << conetNode << ", VME base address " << std::hex << vmeBaseAddr;
		throw Exception(errorCode, s.str());
	}

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

uint32_t Handle::readRegister(uint32_t const reg) {

	uint32_t regData;
	auto const res = CAEN_DGTZ_ReadRegister(handle, reg, &regData);
	if (CAEN_DGTZ_Success != res) {
		std::stringstream s;
		s << "reading register 0x" << std::hex << reg;
		throw Exception(res, s.str());
	}

}

void Handle::writeRegister(uint32_t const reg, uint32_t const regData) {

	auto const res = CAEN_DGTZ_WriteRegister(handle, reg, regData);
	if (CAEN_DGTZ_Success != res) {
		std::stringstream s;
		s << "writing " <<  std::hex << regData << " to register 0x" << std::hex << reg;
		throw Exception(res, s.str());
	}

}

}

