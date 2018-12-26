#ifndef	__fe_v1720_fe_v1720_cxx__
#define	__fe_v1720_fe_v1720_cxx__

#include <cstdint>
#include <midas.h>
#include <caen/v1720.hxx>
#include <util/types.hxx>

constexpr std::size_t calculateEventSize(unsigned const numOfActiveChannels,
		unsigned const recordLength) {

	return sizeof(EVENT_HEADER)	// MIDAS event header
			// global bank header
			+ sizeof(BANK_HEADER)
			// information bank
			+ ALIGN8(sizeof(util::InfoBank)) + sizeof(BANK32)
			// DC offset bank
			+ ALIGN8(sizeof(uint16_t) * caen::v1720::NUM_OF_CHANNELS)
			+ sizeof(BANK32)
			// waveform banks
			+ numOfActiveChannels
					* (ALIGN8(recordLength * sizeof(uint16_t)) + sizeof(BANK32));

}

#endif	//	__fe_v1720_fe_v1720_cxx__
