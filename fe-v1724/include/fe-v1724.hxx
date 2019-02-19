#ifndef	__fe_v1724_fe_v1724_hxx__
#define	__fe_v1724_fe_v1724_hxx__

#include <cstdint>
#include <caen/v1724.hxx>
#include <util/types.hxx>

namespace fe {

namespace v1724 {

#define EQUIP_NAME "v1724"

constexpr char equipName[] = EQUIP_NAME;
constexpr char displayName[] = "V1724";

constexpr std::size_t calculateEventSize(unsigned const numOfActiveChannels,
		unsigned const recordLength) {

	using util::calcEventSize;
	using util::calcBankSize;

	return calcEventSize(
			calcBankSize(sizeof(util::InfoBank)) // information bank
					+ calcBankSize(sizeof(util::TriggerBank)) // trigger info bank
					+ calcBankSize(
							sizeof(uint16_t) * caen::v1724::NUM_OF_CHANNELS) // DC offset bank
					+ numOfActiveChannels
							* calcBankSize(sizeof(uint16_t) * recordLength) // waveform banks
									);

}

constexpr uint32_t MAX_NUM_OF_EVENTS = 100;

constexpr int EVID = 1;

constexpr uint32_t MAX_EVENT_SIZE = calculateEventSize(
		caen::v1724::NUM_OF_CHANNELS, caen::v1724::MAX_RECORD_LENGTH);

namespace settings {

constexpr char waveformLength[] = "waveform_length";
constexpr char enabledChannels[] = "channel_enabled";
constexpr char channelDcOffset[] = "channel_dc_offset";
constexpr char triggerChannel[] = "trigger_channel";
constexpr char triggerThreshold[] = "trigger_threshold";
constexpr char triggerRaisingPolarity[] = "trigger_raising_polarity";
constexpr char preTriggerLength[] = "pre_trigger_length";

}

}

}

#endif	//	__fe_v1724_fe_v1724_hxx__
