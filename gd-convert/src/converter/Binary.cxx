#include <converter/Binary.hxx>
#include <util/caen/V1720InfoRawData.hxx>
#include <util/caen/V1724InfoRawData.hxx>

namespace gdc {

namespace converter {

std::string Binary::Name() {

	return "binary";

}

void Binary::ProcessMidasEvent(std::ostream& dest,
		TDataContainer & dataContainer) {

	{
		using util::caen::V1720InfoRawData;

		auto const v1720Info = dataContainer.GetEventData < V1720InfoRawData
				> (V1720InfoRawData::bankName());
		if (v1720Info) {
			ProcessMidasEvent(dest, dataContainer, *v1720Info);
			return;
		}
	}

	{
		using util::caen::V1724InfoRawData;

		auto const v1724Info = dataContainer.GetEventData < V1724InfoRawData
				> (V1724InfoRawData::bankName());
		if (v1724Info) {
			ProcessMidasEvent(dest, dataContainer, *v1724Info);
			return;
		}
	}

}

std::string Binary::FileExtension() const {

	return ".data";

}

std::ios_base::openmode Binary::FileMode() const {

	return std::ios_base::out | std::ios::binary;

}

void Binary::ProcessMidasEvent(std::ostream& dest,
		TDataContainer& dataContainer,
		util::caen::DigitizerInfoRawData const& info) {

}

}

}
