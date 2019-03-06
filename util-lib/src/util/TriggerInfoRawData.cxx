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

TriggerBank const& TriggerInfoRawData::info(
		uint8_t const triggerChannel) const {

	return reinterpret_cast<TriggerBank const*>(GetData16())[triggerChannel];

}

uint8_t TriggerInfoRawData::triggerChannel(uint8_t const triggerChannel) const {

	return static_cast<uint8_t>(info(triggerChannel).triggerChannel.bits.no);

}

uint16_t TriggerInfoRawData::triggerThreshold(
		uint8_t const triggerChannel) const {

	return static_cast<uint16_t>(info(triggerChannel).triggerThreshold);

}

bool TriggerInfoRawData::triggerRising(uint8_t const triggerChannel) const {

	return info(triggerChannel).triggerInfo.bits.rising == 0 ? false : true;

}

}
