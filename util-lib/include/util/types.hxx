#ifndef	__frontend_util_lib_types_hxx__
#define	__frontend_util_lib_types_hxx__

#include <cstdint>
#include <midas.h>

namespace util {

constexpr std::size_t calcEventSize(std::size_t const banksSize) {

	return sizeof(EVENT_HEADER)	// MIDAS event header
	// global bank header
			+ sizeof(BANK_HEADER)
			// data banks
			+ banksSize;

}

constexpr std::size_t calcBankSize(std::size_t const dataSize) {

	return ALIGN8(dataSize) + sizeof(BANK32);

}

typedef struct
	__attribute__((packed)) {

		uint16_t triggerChannel;
		uint16_t triggerThreshold;
		uint16_t triggerRising;
		uint16_t reserved;

	} TriggerBank;

typedef struct
	__attribute__((packed)) {

		uint32_t boardId;
		uint32_t channelMask;
		uint32_t eventCounter;
		uint32_t timeStampLo;
		uint32_t timeStampHi;
		uint32_t frontendIndex;
		uint32_t preTriggerLength;
		uint32_t triggerMode;

	} InfoBank;

}

#endif	//	__frontend_util_lib_types_hxx__
