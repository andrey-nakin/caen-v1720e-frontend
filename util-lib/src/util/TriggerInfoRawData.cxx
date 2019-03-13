#include "util/TriggerInfoRawData.hxx"

namespace util {

TriggerInfoRawData::TriggerInfoRawData(int const bklen, int const bktype,
		const char* const name, void * const pdata) :
		TGenericData(bklen, bktype, name, pdata) {

}

const char* TriggerInfoRawData::bankName() {

	return "TRIG";

}

int TriggerInfoRawData::numOfChannels() const {

	return GetSize() * sizeof(uint16_t) / sizeof(TriggerBank);

}

TriggerBank const* TriggerInfoRawData::info(int const idx) const {

	return reinterpret_cast<TriggerBank const*>(GetData16()) + idx;

}

TriggerBank const* TriggerInfoRawData::channelInfo(
		channelno_type const ch) const {

	for (int i = 0, last = numOfChannels(); i < last; i++) {
		auto const inf = info(i);
		if (channel(*inf) == ch) {
			return inf;
		}
	}

	return nullptr;

}

TriggerInfoRawData::channelno_type TriggerInfoRawData::channel(
		TriggerBank const& ti) {

	return ti.triggerChannel.bits.no;

}

uint16_t TriggerInfoRawData::threshold(TriggerBank const& ti) {

	return ti.triggerThreshold;

}

bool TriggerInfoRawData::rising(TriggerBank const& ti) {

	return ti.triggerInfo.bits.rising == 0 ? false : true;

}

}
