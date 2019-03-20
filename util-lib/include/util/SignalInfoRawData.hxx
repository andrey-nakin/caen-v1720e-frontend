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
	SignalInfoBank const* info(uint8_t channelNo) const;

	static uint32_t length(SignalInfoBank const&);
	static uint32_t frontLength(SignalInfoBank const&);
	static uint8_t timeTriggers(SignalInfoBank const&);
	static int16_t threshold(SignalInfoBank const&);
	static bool rising(SignalInfoBank const&);
	static bool triggerDisabled(SignalInfoBank const&, uint8_t triggerChannel);
	static bool timeTrigger(SignalInfoBank const&, uint8_t triggerChannel);

};

}
