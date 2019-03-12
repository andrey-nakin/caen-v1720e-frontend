#pragma once

#include <cstdio>
#include <cstdint>
#include <midas/fe-api.hxx>
#include <caen/v1720.hxx>
#include <fe/caen/DigitizerFrontend.hxx>
#include <util/caen/V1720InfoRawData.hxx>

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

	uint32_t getMaxSampleValue() const override {

		return ::caen::v1720::MAX_SAMPLE_VALUE;

	}

	uint32_t postTriggerRegisterValue(uint32_t const recordLength,
			uint32_t const preTriggerLength) const override {

		return (recordLength - preTriggerLength) / 4;

	}

	std::size_t calculateEventSize(unsigned const numOfActiveChannels,
			unsigned const recordLength) const override {

		return fe::caen::v1720::calculateEventSize(numOfActiveChannels,
				recordLength);

	}

	char const* infoBankName() const override {

		return util::caen::V1720InfoRawData::bankName();

	}

	void checkBoardInfo(CAEN_DGTZ_BoardInfo_t const& boardInfo) override {

		if (boardInfo.Model != CAEN_DGTZ_V1720) {
			throw ::caen::Exception(CAEN_DGTZ_GenericError,
					"The device is not CAEN V1720");
		}

		int majorNumber, minorNumber;
		sscanf(boardInfo.AMC_FirmwareRel, "%d", &majorNumber);
		if (majorNumber >= 128) {
			throw ::caen::Exception(CAEN_DGTZ_GenericError,
					"This digitizer has a DPP firmware");
		}

		sscanf(boardInfo.ROC_FirmwareRel, "%d.%d", &majorNumber, &minorNumber);
		if (majorNumber < 4 || (majorNumber == 4 && minorNumber < 6)) {
			throw ::caen::Exception(CAEN_DGTZ_GenericError,
					"ROC FPGA firmware 4.6 or higher required");
		}
	}

};

}

}

}
