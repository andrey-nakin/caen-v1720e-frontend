#pragma once

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

		union {
			struct {
				uint16_t no :4;
				uint16_t :12;	//	reserved
			} bits;
			uint16_t raw;
		} triggerChannel;

		uint16_t triggerThreshold;

		union {
			struct {
				uint16_t rising :1;
				uint16_t master :1;
				uint16_t :14;	//	reserved
			} bits;
			uint16_t raw;
		} triggerInfo;

		uint16_t reserved;

	} TriggerBank;

	static_assert(sizeof(TriggerBank) == sizeof(uint16_t) * 4, "TriggerBank size is not correct");

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

		static_assert(sizeof(InfoBank) == sizeof(uint32_t) * 8, "InfoBank size is not correct");

		void fillTriggerInfo(TriggerBank& bank, uint8_t channel,
				uint32_t threshold, bool rising, bool master);

		}
