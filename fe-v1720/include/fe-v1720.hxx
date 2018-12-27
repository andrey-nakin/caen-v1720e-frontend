#ifndef	__fe_v1720_fe_v1720_cxx__
#define	__fe_v1720_fe_v1720_cxx__

#include <cstdint>
#include <caen/v1720.hxx>
#include <util/types.hxx>

#define EQUIP_NAME "v1720"

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

#endif	//	__fe_v1720_fe_v1720_cxx__
