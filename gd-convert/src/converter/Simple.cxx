#include <sstream>
#include <fstream>
#include <iomanip>
#include <util/caen/V1720InfoRawData.hxx>
#include <util/caen/V1724InfoRawData.hxx>
#include <util/TWaveFormRawData.hxx>
#include <converter/Simple.hxx>

namespace gdc {

namespace converter {

Simple::Simple() {

}

std::string Simple::Name() {

	return "simple";

}

void Simple::ProcessMidasEvent(TDataContainer & dataContainer) {

	{
		using util::caen::V1720InfoRawData;

		auto const v1720Info = dataContainer.GetEventData < V1720InfoRawData
				> (V1720InfoRawData::bankName());
		if (v1720Info) {
			ProcessMidasEvent(dataContainer, *v1720Info);
			return;
		}
	}

	{
		using util::caen::V1724InfoRawData;

		auto const v1724Info = dataContainer.GetEventData < V1724InfoRawData
				> (V1724InfoRawData::bankName());
		if (v1724Info) {
			ProcessMidasEvent(dataContainer, *v1724Info);
			return;
		}
	}

}

void Simple::ProcessMidasEvent(TDataContainer& dataContainer,
		util::caen::DigitizerInfoRawData const& info) {

	using util::TWaveFormRawData;

	std::ofstream dest(ConstructName(info));

	std::size_t recordLength;

	dest << "# INFO";
	for (uint8_t ch = 0; ch < 8; ch++) {
		dest << "\tCH" << static_cast<int>(ch);

		if (info.channelIncluded(ch)) {
			auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
					> (TWaveFormRawData::bankName(ch));
			if (wfRaw) {
				recordLength = std::max(recordLength, wfRaw->numOfSamples());
			}
		}

	}
	dest << "\n";

	for (std::size_t i = 0; i < recordLength; i++) {
		for (uint8_t ch = 0; ch < 8; ch++) {
			dest << '\t';

			if (info.channelIncluded(ch)) {
				auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
						> (TWaveFormRawData::bankName(ch));
				if (wfRaw) {
					if (i < wfRaw->numOfSamples()) {
						dest << *(wfRaw->begin() + i);
					}
				}
			}

		}
		dest << "\n";
	}

}

std::string Simple::ConstructName(
		util::caen::DigitizerInfoRawData const& info) {

	std::stringstream s;
	s << "event" << std::setfill('0') << std::setw(6)
			<< info.info().eventCounter << ".txt";
	return s.str();

}

}

}
