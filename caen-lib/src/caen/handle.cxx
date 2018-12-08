#include <sstream>
#include <CAENDigitizer.h>
#include <caen/handle.hxx>

namespace caen {

constexpr Handle::handle_type INVALID_HANDLE_VALUE = -1;

Handle::Handle(int const linkNum, int const conetNode,
		::uint32_t const vmeBaseAddr) :
		handle(INVALID_HANDLE_VALUE) {

	commandE(
			[linkNum, conetNode, vmeBaseAddr]() {
				std::stringstream s;
				s << "opening device on link # " << linkNum << ", conet node # " << conetNode << ", VME base address " << std::hex << vmeBaseAddr;
				return s.str();
			},
			[this, linkNum, conetNode, vmeBaseAddr]() {
				return CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, linkNum, conetNode, vmeBaseAddr, &handle);
			});

}

Handle::Handle(Handle&& h) :
		handle(INVALID_HANDLE_VALUE) {

	this->handle = h.handle;
	h.handle = INVALID_HANDLE_VALUE;

}

Handle::~Handle() {

	if (INVALID_HANDLE_VALUE != handle) {
		CAEN_DGTZ_CloseDigitizer(handle);
	}

}

uint32_t Handle::readRegister(uint32_t const reg) {

	uint32_t regData;
	hCommandE([reg]() {
		std::stringstream s;
		s << "reading register 0x" << std::hex << reg;
		return s.str();
	}, [reg, &regData](handle_type handle) {
		return CAEN_DGTZ_ReadRegister(handle, reg, &regData);
	});
	return regData;

}

void Handle::writeRegister(uint32_t const reg, uint32_t const regData) {

	hCommandE(
			[reg, regData]() {
				std::stringstream s;
				s << "writing " << std::hex << regData << " to register 0x" << std::hex << reg;
				return s.str();
			}, [reg, regData](handle_type handle) {
				return CAEN_DGTZ_WriteRegister(handle, reg, regData);
			});

}

}

