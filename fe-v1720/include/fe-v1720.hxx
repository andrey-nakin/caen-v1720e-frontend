#ifndef	__fe_v1720_fe_v1720_cxx__
#define	__fe_v1720_fe_v1720_cxx__

#include <cstdint>
#include <caen/v1720.hxx>
#include <util/types.hxx>

namespace fe {

namespace v1720 {

#define EQUIP_NAME "v1720"

constexpr char equipName[] = EQUIP_NAME;
constexpr char displayName[] = "V1720";

constexpr std::size_t calculateEventSize(unsigned const numOfActiveChannels,
		unsigned const recordLength) {

	using util::calcEventSize;
	using util::calcBankSize;

	return calcEventSize(
			calcBankSize(sizeof(util::InfoBank)) // information bank
					+ calcBankSize(
							sizeof(uint16_t) * caen::v1720::NUM_OF_CHANNELS) // DC offset bank
					+ numOfActiveChannels
							* calcBankSize(sizeof(uint16_t) * recordLength) // waveform banks
									);

}

constexpr uint32_t MAX_NUM_OF_EVENTS = 100;

constexpr int EVID = 1;

constexpr uint32_t MAX_EVENT_SIZE = calculateEventSize(
		caen::v1720::NUM_OF_CHANNELS, caen::v1720::MAX_RECORD_LENGTH);

namespace settings {

constexpr char waveformLength[] = "waveform_length";
constexpr char enabledChannels[] = "channel_enabled";
constexpr char channelDcOffset[] = "channel_dc_offset";
constexpr char triggerChannel[] = "trigger_channel";
constexpr char triggerThreshold[] = "trigger_threshold";
constexpr char triggerRaisingPolarity[] = "trigger_raising_polarity";
constexpr char preTriggerLength[] = "pre_trigger_length";
constexpr char linkNum[] = "link_num";
constexpr char conetNode[] = "conet_node";
constexpr char vmeBaseAddr[] = "vme_base_addr";

}

}

}

#endif	//	__fe_v1720_fe_v1720_cxx__
