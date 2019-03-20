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

int SignalInfoRawData::triggerChannel(SignalInfoBank const& si) {

	// TODO
	return -1;
//	return si.pattern.bits.triggerChannel < 0 ?
//			-1 : si.pattern.bits.triggerChannel;

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

}
