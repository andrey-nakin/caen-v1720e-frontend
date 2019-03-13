#include <util/SignalInfoRawData.hxx>

namespace util {

char const* SignalInfoRawData::BANK_NAME = "SGNL";

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

	return si.pattern.bits.triggerChannel == static_cast<uint8_t>(-1) ?
			-1 : si.pattern.bits.triggerChannel;

}

int16_t SignalInfoRawData::threshold(SignalInfoBank const& si) {

	return si.pattern.bits.threshold;

}

bool SignalInfoRawData::rising(SignalInfoBank const& si) {

	return si.pattern.bits.rising ? true : false;

}

}
