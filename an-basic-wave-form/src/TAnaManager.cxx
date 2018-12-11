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

	auto const v1720Info = dataContainer.GetEventData < util::V1720InfoRawData
			> (util::V1720InfoRawData::bankName());
	auto const dcOffsets = dataContainer.GetEventData < util::TDcOffsetRawData
			> (util::TDcOffsetRawData::BANK_NAME);

	if (v1720Info && dcOffsets) {
		auto const c = std::unique_ptr < TCanvas > (new TCanvas());

		for (uint8_t channelNo = 0; channelNo < 8; channelNo++) {
			if (v1720Info->channelIncluded(channelNo)) {
				auto const wfRaw = dataContainer.GetEventData
						< util::TWaveFormRawData
						> (util::TWaveFormRawData::bankName(channelNo));
				if (wfRaw) {
					util::V1720WaveForm wf(*wfRaw,
							dcOffsets->dcOffset(channelNo));
					auto const gr = std::unique_ptr < TGraph
							> (new TGraph(wf.size(), wf.getTimePtr(),
									wf.getVoltagePtr()));
					gr->Draw();

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

	// Do little analysis of the V1720 data, as example...
//	if (fV1720Waveform) {
//
//		TV1720RawData *v1720 = dataContainer.GetEventData < TV1720RawData
//				> ("W200");
//
//		if (v1720 && !v1720->IsZLECompressed()) {
//
//			double time[2], ph[2];
//
//			for (int i = 0; i < 2; i++) { // loop first two channels
//
//				TV1720RawChannel channelData = v1720->GetChannelData(i);
//				if (channelData.GetNSamples() <= 0)
//					continue;
//
//				double max_adc_value = -1.0;
//				double max_adc_time = -1;
//				for (int j = 0; j < channelData.GetNSamples(); j++) {
//					double adc = channelData.GetADCSample(j);
//					if (adc > max_adc_value) {
//						max_adc_value = adc;
//						max_adc_time = j * 4.0; // 4ns per bin
//					}
//				}
//				time[i] = max_adc_time;
//				ph[i] = max_adc_value;
//				//std::cout << i << " "  << max_adc_time << " " << max_adc_value << std::endl;
//			}
//			fV1720PHCompare->Fill(ph[0], ph[1]);
//			fV1720TimeCompare->Fill(time[0], time[1]);
//		}
//	}
	return 1;
}

void TAnaManager::BeginRun(int /* transition */, int /* run */,
		int /* time */) {
}

void TAnaManager::EndRun(int /* transition */, int /* run */, int /* time */) {
}

}
