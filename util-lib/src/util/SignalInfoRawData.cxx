#include <util/SignalInfoRawData.hxx>

namespace util {

char const* SignalInfoRawData::BANK_NAME = "SGL0";

SignalInfoRawData::SignalInfoRawData(int const bklen, int const bktype,
		const char* const name, void * const pdata) :
		TGenericData(bklen, bktype, name, pdata) {
}

uint8_t SignalInfoRawData::numOfChannels() const {

	return GetSize() / (sizeof(SignalInfoBank) / sizeof(uint32_t));

}

SignalInfoBank const* SignalInfoRawData::info(uint8_t const channelNo) const {

	return channelNo < numOfChannels() ?
			reinterpret_cast<SignalInfoBank const*>(GetData16()) + channelNo :
			nullptr;

}

uint32_t SignalInfoRawData::length(SignalInfoBank const&si) {

	return si.length;

}

uint32_t SignalInfoRawData::frontLength(SignalInfoBank const& si) {

	return si.frontLength;

}

uint8_t SignalInfoRawData::timeTriggers(SignalInfoBank const& si) {

	return si.pattern.bits.timeTriggers;

}

int16_t SignalInfoRawData::threshold(SignalInfoBank const& si) {

	return si.pattern.bits.threshold;

}

bool SignalInfoRawData::rising(SignalInfoBank const& si) {

	return si.pattern2.bits.rising ? true : false;

}

bool SignalInfoRawData::triggerDisabled(SignalInfoBank const& si,
		uint8_t const triggerChannel) {

	return 0 != (si.pattern.bits.disabledTriggers & (0x01 << triggerChannel));

}

bool SignalInfoRawData::timeTrigger(SignalInfoBank const& si,
		uint8_t const channel) {

	return 0 != (si.pattern.bits.timeTriggers & (0x01 << channel));

}

}
