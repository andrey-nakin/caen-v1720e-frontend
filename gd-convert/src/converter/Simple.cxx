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
			//ProcessMidasEvent(dest, dataContainer, *v1720Info);
			return;
		}
	}

	{
		using util::caen::V1724InfoRawData;

		auto const v1724Info = dataContainer.GetEventData < V1724InfoRawData
				> (V1724InfoRawData::bankName());
		if (v1724Info) {
			//ProcessMidasEvent(dest, dataContainer, *v1724Info);
			return;
		}
	}

}

}

}
