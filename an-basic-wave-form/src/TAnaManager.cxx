#include <util/V1720InfoRawData.hxx>
#include <util/TDcOffsetRawData.hxx>
#include <util/TWaveFormRawData.hxx>
#include "TAnaManager.hxx"

namespace bwf {

TAnaManager::TAnaManager() {

}

int TAnaManager::ProcessMidasEvent(TDataContainer& dataContainer) {

	auto const info = dataContainer.GetEventData < util::V1720InfoRawData
			> (util::V1720InfoRawData::bankName());
	if (info) {
		std::cout << "V1720 INFO bank detected, " << info->info().timeStamp
				<< std::endl;
	}

	auto const dcOffsets = dataContainer.GetEventData < util::TDcOffsetRawData
			> (util::TDcOffsetRawData::BANK_NAME);
	if (dcOffsets) {
		std::cout << "DC offset bank detected, ch[7, 8] = "
				<< dcOffsets->dcOffset(7) << ", " << dcOffsets->dcOffset(-1)
				<< ", size=" << dcOffsets->GetSize() << std::endl;
	}

	auto const wf00 = dataContainer.GetEventData < util::TWaveFormRawData
			> (util::TWaveFormRawData::bankName(0));
	if (wf00) {
		std::cout << "WF00 offset bank detected, size = " << wf00->GetSize()
				<< std::endl;
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
