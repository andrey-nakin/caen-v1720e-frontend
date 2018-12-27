#include <sstream>
#include <fstream>
#include <memory>
#include <TGraph.h>
#include <TCanvas.h>
#include <util/V1720InfoRawData.hxx>
#include <util/TDcOffsetRawData.hxx>
#include <util/TWaveFormRawData.hxx>
#include <util/V1720WaveForm.hxx>
#include "TAnaManager.hxx"

namespace bwf {

TAnaManager::TAnaManager() {

}

int TAnaManager::ProcessMidasEvent(TDataContainer& dataContainer) {

	using util::V1720InfoRawData;
	using util::TDcOffsetRawData;
	using util::TWaveFormRawData;

	auto const v1720Info = dataContainer.GetEventData < V1720InfoRawData
			> (V1720InfoRawData::bankName());
	auto const dcOffsets = dataContainer.GetEventData < TDcOffsetRawData
			> (TDcOffsetRawData::BANK_NAME);

	if (v1720Info && dcOffsets) {
		std::unique_ptr<TCanvas> const c(new TCanvas());

		for (uint8_t channelNo = 0; channelNo < 8; channelNo++) {
			if (v1720Info->channelIncluded(channelNo)) {
				auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
						> (TWaveFormRawData::bankName(channelNo));
				if (wfRaw) {
					util::V1720WaveForm wf(*wfRaw,
							dcOffsets->dcOffset(channelNo));
					TGraph gr(wf.size(), wf.getTimePtr(), wf.getVoltagePtr());
					gr.Draw();

//					std::stringstream s;
//					s << "wf-data-" << std::setfill('0') << std::setw(4)
//							<< v1720Info->info().eventCounter << '-'
//							<< std::setw(1) << static_cast<int>(channelNo)
//							<< ".txt";
//					std::ofstream f(s.str());

//					auto times = wf.getTimes();
//					auto voltages = wf.getVoltages();
//					for (std::size_t i = 0; i < wf.size(); i++) {
//						f << times[i] << '\t' << voltages[i] << '\n';
//					}
				}
			}

		}

		std::stringstream s;
		s << "wf" << std::setfill('0') << std::setw(6)
				<< v1720Info->info().eventCounter << ".png";
		auto const name = s.str();
		c->SaveAs(name.c_str());
	}

	return 1;
}

void TAnaManager::BeginRun(int /* transition */, int /* run */,
		int /* time */) {
}

void TAnaManager::EndRun(int /* transition */, int /* run */, int /* time */) {
}

}
