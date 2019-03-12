#pragma once

#include <cstdint>
#include <TGenericData.hxx>
#include "types.hxx"

namespace util {

class SignalInfoRawData: public TGenericData {
public:

	static char const* BANK_NAME;

	SignalInfoRawData(int bklen, int bktype, const char* name, void *pdata);
	uint8_t numOfChannels() const;
	SignalInfoBank const& info(uint8_t channelNo) const;

	uint32_t length(uint8_t channelNo) const;
	uint32_t frontLength(uint8_t channelNo) const;
	uint8_t triggerChannel(uint8_t channelNo) const;
	bool rising(uint8_t channelNo) const;

};

}
