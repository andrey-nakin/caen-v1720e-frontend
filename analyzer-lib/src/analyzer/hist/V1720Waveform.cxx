#include <sstream>
#include <caen/v1720.hxx>
#include <midas/odb.hxx>
#include <fe-v1720.hxx>
#include <math/DiffContainer.hxx>
#include <math/StatAccum.hxx>
#include <math/PeakFinder.hxx>
#include <math/EdgeFinder.hxx>
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
		auto const feIndex = frontendIndex(v1720Info->info().frontendIndex);
		auto const edgePosition = FindEdgeDistance(dataContainer, v1720Info);

		for (uint8_t channelNo = 0; channelNo < caen::v1720::NUM_OF_CHANNELS;
				channelNo++) {
			if (v1720Info->channelIncluded(channelNo)) {
				auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
						> (TWaveFormRawData::bankName(channelNo));
				if (wfRaw) {
					auto const numOfSamples = wfRaw->numOfSamples();
					if (numOfSamples > 0) {
						// draw raw waveform
						auto &h = GetWaveformHist(feIndex, channelNo,
								numOfSamples);
						SetData(h, wfRaw->begin(), wfRaw->end());

						AnalyzeWaveform(feIndex, channelNo, numOfSamples,
								edgePosition, wfRaw);
					}
				}
			}
		}
	}

}

void V1720Waveform::AnalyzeWaveform(INT const feIndex, uint8_t const channelNo,
		std::size_t const numOfSamples,
		util::TWaveFormRawData::difference_type const edgePosition,
		util::TWaveFormRawData const* const wfRaw) {

	auto const wfStat = math::MakeStatAccum(wfRaw->begin(), wfRaw->end());
	auto const wfDiff = math::MakeDiffContainer<int16_t>(wfRaw->begin(),
			wfRaw->end(), frontLength);
	auto const diffStat = math::MakeStatAccum(std::begin(wfDiff),
			std::end(wfDiff));

	auto const t = diffStat.GetStdScaled < util::TWaveFormRawData::value_type
			> (threshold);

	auto const hasPeak =
			rising ? diffStat.GetMaxValue() >= t : diffStat.GetMinValue() <= -t;
	auto const wfBegin = std::next(wfRaw->begin(), edgePosition);
	auto const wfEnd = wfRaw->end();

	if (hasPeak && wfBegin < wfEnd) {
		auto &ph = GetPositionHist(feIndex, channelNo, numOfSamples);
		auto &ah = GetAmplitudeHist(feIndex, channelNo,
				caen::v1720::NUM_OF_SAMPLE_VALUES);

		auto pf = math::MakePeakFinder(rising, wfBegin, wfEnd, frontLength, t,
				peakLength);
		while (pf.HasNext()) {
			auto const i = pf.GetNext();
			auto const position = std::distance(wfBegin, i);
			auto const zeroLevel = wfStat.GetRoughMean();
			decltype(zeroLevel) const ampAdjusted =
					rising ? *i - zeroLevel : zeroLevel - *i;
			auto const amplitude = std::min(ampAdjusted,
					caen::v1720::MAX_SAMPLE_VALUE);

			ph.AddBinContent(position);
			ah.AddBinContent(amplitude);
		}
	}

}

util::TWaveFormRawData::difference_type V1720Waveform::FindEdgeDistance(
		TDataContainer &dataContainer, util::V1720InfoRawData const*v1720Info) {

	using util::TWaveFormRawData;

	if (v1720Info->hasTriggerSettings() && v1720Info->triggerMode() == 0) {
		auto const triggerChannel = v1720Info->triggerChannel();

		if (v1720Info->channelIncluded(triggerChannel)) {
			auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
					> (TWaveFormRawData::bankName(triggerChannel));
			if (wfRaw) {
				return math::FindEdgeDistance(v1720Info->triggerRising(),
						v1720Info->triggerThreshold(), wfRaw->begin(),
						wfRaw->end());
			}
		}
	}

	return 0;

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
