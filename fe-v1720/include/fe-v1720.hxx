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
					+ calcBankSize(sizeof(util::TriggerBank)) // trigger info bank
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

}

}

#endif	//	__fe_v1720_fe_v1720_cxx__
