#include <util/types.hxx>

namespace util {

void fillTriggerInfo(TriggerBank& bank, uint8_t const channel,
		uint32_t const threshold, bool const rising) {

	bank.triggerChannel.raw = 0;
	bank.triggerChannel.bits.no = channel;
	bank.triggerThreshold = static_cast<uint16_t>(threshold);
	bank.triggerInfo.raw = 0;
	bank.triggerInfo.bits.rising = rising ? 1 : 0;
	bank.reserved = static_cast<uint16_t>(threshold >> 16);

}

void fillSignalInfo(SignalInfoBank& bank, uint32_t const length,
		uint32_t const frontLength, uint8_t const timeTriggers,
		uint8_t const disabledTriggers, int16_t const threshold,
		bool const rising, uint32_t const maxTime) {

	bank.length = length;
	bank.frontLength = frontLength;
	bank.pattern.raw = 0;
	bank.pattern.bits.timeTriggers = timeTriggers;
	bank.pattern.bits.disabledTriggers = disabledTriggers;
	bank.pattern.bits.threshold = threshold;
	bank.pattern2.bits.rising = rising ? 1 : 0;
	bank.maxTime = maxTime;

}

}
