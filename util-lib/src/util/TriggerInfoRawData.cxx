#include "util/TriggerInfoRawData.hxx"

namespace util {

TriggerInfoRawData::TriggerInfoRawData(int const bklen, int const bktype,
		const char* const name, void * const pdata) :
		TGenericData(bklen, bktype, name, pdata) {

}

const char* TriggerInfoRawData::bankName() {

	return "TRIG";

}

TriggerBank const& TriggerInfoRawData::info() const {

	return *reinterpret_cast<TriggerBank const*>(GetData16());

}

uint8_t TriggerInfoRawData::triggerChannel() const {

	return static_cast<uint8_t>(info().triggerChannel);

}

uint16_t TriggerInfoRawData::triggerThreshold() const {

	return static_cast<uint16_t>(info().triggerThreshold);

}

bool TriggerInfoRawData::triggerRising() const {

	return info().triggerRising == 0 ? false : true;

}

}
