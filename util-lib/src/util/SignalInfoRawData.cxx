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

SignalInfoBank const& SignalInfoRawData::info(uint8_t const channelNo) const {

	return reinterpret_cast<SignalInfoBank const*>(GetData16())[channelNo];

}

uint32_t SignalInfoRawData::length(uint8_t const channelNo) const {

	return channelNo < numOfChannels() ? info(channelNo).length : 0;

}

uint32_t SignalInfoRawData::frontLength(uint8_t const channelNo) const {

	return channelNo < numOfChannels() ? info(channelNo).frontLength : 0;

}

uint8_t SignalInfoRawData::triggerChannel(uint8_t const channelNo) const {

	return channelNo < numOfChannels() ?
			info(channelNo).pattern.bits.triggerChannel : 0;

}

bool SignalInfoRawData::rising(uint8_t const channelNo) const {

	return channelNo < numOfChannels() ?
			(info(channelNo).pattern.bits.rising ? true : false) : false;

}

}
