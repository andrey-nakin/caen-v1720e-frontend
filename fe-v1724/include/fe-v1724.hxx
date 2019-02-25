#ifndef	__fe_v1724_fe_v1724_hxx__
#define	__fe_v1724_fe_v1724_hxx__

#include <cstdint>
#include <midas/fe-api.hxx>
#include <caen/v1724.hxx>
#include <util/types.hxx>
#include <fe/caen/DigitizerFrontend.hxx>
#include <util/V1720InfoRawData.hxx>

namespace fe {

namespace caen {

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
							sizeof(uint16_t) * ::caen::v1724::NUM_OF_CHANNELS) // DC offset bank
					+ numOfActiveChannels
							* calcBankSize(sizeof(uint16_t) * recordLength) // waveform banks
									);

}

constexpr uint32_t MAX_EVENT_SIZE = calculateEventSize(
		::caen::v1724::NUM_OF_CHANNELS, ::caen::v1724::MAX_RECORD_LENGTH);

class V1724DigitizerFrontend: public DigitizerFrontend {
protected:

	uint32_t getMaxRecordLength() const override {

		return ::caen::v1724::MAX_RECORD_LENGTH;

	}

	std::size_t calculateEventSize(unsigned const numOfActiveChannels,
			unsigned const recordLength) const override {

		return fe::caen::v1724::calculateEventSize(numOfActiveChannels,
				recordLength);

	}

	char const* infoBankName() const override {

		// TODO
		return util::V1720InfoRawData::bankName();

	}

	void checkBoardInfo(CAEN_DGTZ_BoardInfo_t const& boardInfo) override {

		if (boardInfo.Model != CAEN_DGTZ_V1724) {
			throw ::caen::Exception(CAEN_DGTZ_GenericError,
					"The device is not CAEN V1724");
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

#endif	//	__fe_v1724_fe_v1724_hxx__
