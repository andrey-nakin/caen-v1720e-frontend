#pragma once

#include <cstdio>
#include <cstdint>
#include <midas.h>
#include <caen/v1720.hxx>
#include <util/types.hxx>
#include <fe/caen/DigitizerFrontend.hxx>
#include <util/V1720InfoRawData.hxx>

#ifndef NEED_NO_EXTERN_C
extern "C" {
#endif

INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();
INT poll_event(INT source, INT count, BOOL test);
INT interrupt_configure(INT cmd, INT source, PTYPE adr);
int readEvent(char * const pevent, int const off);

#ifndef NEED_NO_EXTERN_C
}
#endif

namespace fe {

namespace caen {

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
							sizeof(uint16_t) * ::caen::v1720::NUM_OF_CHANNELS) // DC offset bank
					+ numOfActiveChannels
							* calcBankSize(sizeof(uint16_t) * recordLength) // waveform banks
									);

}

constexpr uint32_t MAX_EVENT_SIZE = calculateEventSize(
		::caen::v1720::NUM_OF_CHANNELS, ::caen::v1720::MAX_RECORD_LENGTH);

class V1720DigitizerFrontend: public DigitizerFrontend {
protected:

	uint32_t getMaxRecordLength() const override {

		return ::caen::v1720::MAX_RECORD_LENGTH;

	}

	std::size_t calculateEventSize(CAEN_DGTZ_EventInfo_t const& eventInfo,
			CAEN_DGTZ_UINT16_EVENT_t const& event) const override {

		// count number of active channels
		unsigned numOfActiveChannels = 0, recordLength = 0;
		for (unsigned i = 0; i < boardInfo.Channels; i++) {
			if (eventInfo.ChannelMask & (0x0001 << i)) {
				numOfActiveChannels++;
				recordLength = std::max(recordLength, event.ChSize[i]);
			}
		}

		return fe::caen::v1720::calculateEventSize(numOfActiveChannels,
				recordLength);

	}

	char const* infoBankName() const override {

		return util::V1720InfoRawData::bankName();

	}

	void checkBoardInfo(CAEN_DGTZ_BoardInfo_t const& boardInfo) override {

		if (boardInfo.Model != CAEN_DGTZ_V1720) {
			throw ::caen::Exception(CAEN_DGTZ_GenericError,
					"The device is not CAEN V1720");
		}

		int majorNumber;
		sscanf(boardInfo.AMC_FirmwareRel, "%d", &majorNumber);
		if (majorNumber >= 128) {
			throw ::caen::Exception(CAEN_DGTZ_GenericError,
					"This digitizer has a DPP firmware");
		}

	}

};

}

}

}
