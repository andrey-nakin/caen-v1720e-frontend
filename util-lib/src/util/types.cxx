#include <util/types.hxx>

namespace util {

void fillTriggerInfo(TriggerBank& bank, uint8_t const channel,
		uint32_t const threshold, bool const rising, bool const master) {

	bank.triggerChannel.raw = 0;
	bank.triggerChannel.bits.no = channel;
	bank.triggerThreshold = static_cast<uint16_t>(threshold);
	bank.triggerInfo.raw = 0;
	bank.triggerInfo.bits.rising = rising ? 1 : 0;
	bank.triggerInfo.bits.master = master ? 1 : 0;
	bank.reserved = static_cast<uint16_t>(threshold >> 16);

}

}
