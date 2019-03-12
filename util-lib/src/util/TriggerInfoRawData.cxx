#include "util/TriggerInfoRawData.hxx"

namespace util {

TriggerInfoRawData::TriggerInfoRawData(int const bklen, int const bktype,
		const char* const name, void * const pdata) :
		TGenericData(bklen, bktype, name, pdata) {

}

const char* TriggerInfoRawData::bankName() {

	return "TRIG";

}

uint8_t TriggerInfoRawData::numOfChannels() const {

	return GetSize() * sizeof(uint16_t) / sizeof(TriggerBank);

}

int8_t TriggerInfoRawData::triggerChannelIndex(uint8_t const channel) const {

	for (uint8_t i = 0, last = numOfChannels(); i < last; i++) {
		if (triggerChannel(i) == channel) {
			return i;
		}
	}

	return -1;

}

TriggerBank const& TriggerInfoRawData::info(uint8_t const idx) const {

	return reinterpret_cast<TriggerBank const*>(GetData16())[idx];

}

uint8_t TriggerInfoRawData::triggerChannel(uint8_t const idx) const {

	return static_cast<uint8_t>(info(idx).triggerChannel.bits.no);

}

uint16_t TriggerInfoRawData::triggerThreshold(uint8_t const idx) const {

	return static_cast<uint16_t>(info(idx).triggerThreshold);

}

bool TriggerInfoRawData::triggerRising(uint8_t const idx) const {

	return info(idx).triggerInfo.bits.rising == 0 ? false : true;

}

}
