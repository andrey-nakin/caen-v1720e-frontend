#include <sstream>
#include <caen/v1720.hxx>
#include <midas/odb.hxx>
#include <fe-v1720.hxx>
#include <util/V1720InfoRawData.hxx>
#include <util/TWaveFormRawData.hxx>
#include <math/DiffContainer.hxx>
#include <math/StatAccum.hxx>
#include <math/PeakFinder.hxx>
#include "analyzer/hist/V1720Waveform.hxx"

namespace analyzer {

namespace hist {

V1720Waveform::V1720Waveform(VirtualOdb* const anOdb) :
		AbstractWaveform(anOdb, fe::v1720::equipName, fe::v1720::displayName,
				caen::v1720::nsPerSample<ns_per_sample_type>()), minFront(14), frontLength(
				3), peakLength(16), threshold(7.0), rising(false) {

}

void V1720Waveform::UpdateHistograms(TDataContainer &dataContainer) {

	using util::V1720InfoRawData;
	using util::TWaveFormRawData;

	auto const v1720Info = dataContainer.GetEventData < V1720InfoRawData
			> (V1720InfoRawData::bankName());
	if (v1720Info) {
		for (unsigned channelNo = 0; channelNo < caen::v1720::NUM_OF_CHANNELS;
				channelNo++) {
			if (v1720Info->channelIncluded(channelNo)) {
				auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
						> (TWaveFormRawData::bankName(channelNo));
				if (wfRaw) {
					auto const numOfSamples = wfRaw->numOfSamples();
					if (numOfSamples > 0) {
						// retrieve waveform
						auto const wf = wfRaw->waveForm();
						auto const feIndex = frontendIndex(
								v1720Info->info().frontendIndex);
						auto const wfSa = math::MakeStatAccum(wf,
								wf + numOfSamples);

						{
							// draw raw waveform
							auto &h = GetWaveformHist(feIndex, channelNo,
									numOfSamples);
							SetData(h, wf, wf + numOfSamples);
						}

						auto const dc = math::MakeDiffContainer<int16_t>(wf,
								wf + numOfSamples, frontLength);
						auto const dcSa = math::MakeStatAccum(std::begin(dc),
								std::end(dc));

//						if (files.end() == files.find(channelNo)) {
//							std::stringstream s;
//							s << "channel." << channelNo << ".txt";
//							std::string name = s.str();
//							files[channelNo] = std::unique_ptr < std::ofstream
//									> (new std::ofstream(name));
//							*files[channelNo] << "diff" << std::endl;
//						}

						auto const t = dcSa.GetStdScaled
								< util::TWaveFormRawData::value_type
								> (threshold);

//						{
//							auto& s = *files[channelNo];
//							std::for_each(std::begin(dc), std::end(dc),
//									[&s, t](int16_t d) {
//										s << d << '\t' << t << '\n';
//									});
//						}

						auto const hasPeak =
								rising ?
										dcSa.GetMaxValue() >= t :
										dcSa.GetMinValue() <= -t;

//						static int cnt = 0;
//						if (channelNo == 1 && cnt++ < 10) {
//							std::stringstream s;
//							s << "waveform." << channelNo << "."
//									<< v1720Info->info().eventCounter << '.'
//									<< (hasPeak ? "yes" : "no") << ".txt";
//							std::ofstream f(s.str());
//
//							std::for_each(wf, wf + numOfSamples,
//									[&f](decltype(*wf) s) {
//										f << s << '\n';
//									});
//						}

						if (hasPeak) {
							auto &ph = GetPositionHist(feIndex, channelNo,
									numOfSamples);
							auto &ah = GetAmplitudeHist(feIndex, channelNo,
									caen::v1720::NUM_OF_SAMPLE_VALUES);

							auto pf = math::MakePeakFinder(rising, wf,
									wf + numOfSamples, frontLength, t,
									peakLength);
							while (pf.HasNext()) {
								auto const i = pf.GetNext();
								auto const position = std::distance(wf, i);
								decltype(wfSa.GetRoughMean()) const ampAdjusted =
										rising ?
												*i - wfSa.GetRoughMean() :
												wfSa.GetRoughMean() - *i;
								auto const amplitude = std::min(ampAdjusted,
										caen::v1720::MAX_SAMPLE_VALUE);

								ph.AddBinContent(position);
								ah.AddBinContent(amplitude);
							}
						}
					}
				}
			}
		}
	}

}

//unsigned V1720Waveform::loadWaveformLength(INT const feIndex) {
//
//	auto const hKeyName = odb::equipSettingsKeyName(getBaseEquipName(), feIndex,
//			fe::v1720::settings::waveformLength);
//
//	if (getOdb()->odbReadArraySize(hKeyName.c_str()) <= 0) {
//		return 0;
//	}
//
//	return getOdb()->odbReadUint32(hKeyName.c_str(), 0, 0);
//
//}
//
//std::vector<bool> V1720Waveform::loadEnabledChannels(INT feIndex) {
//
//	std::vector<bool> result;
//	auto const hKeyName = odb::equipSettingsKeyName(getBaseEquipName(), feIndex,
//			fe::v1720::settings::enabledChannels);
//
//	auto const size = getOdb()->odbReadArraySize(hKeyName.c_str());
//	if (size > 0) {
//		result.resize(size);
//
//		for (int i = 0; i < size; i++) {
//			result[i] = getOdb()->odbReadBool(hKeyName.c_str(), i, false);
//		}
//	}
//
//	return result;
//
//}

}

}
