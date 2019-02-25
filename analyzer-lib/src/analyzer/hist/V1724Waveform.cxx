#include <sstream>
#include <caen/v1724.hxx>
#include <midas/odb.hxx>
#include <fe-v1724.hxx>
#include <util/TriggerInfoRawData.hxx>
#include <math/DiffContainer.hxx>
#include <math/StatAccum.hxx>
#include <math/PeakFinder.hxx>
#include <math/EdgeFinder.hxx>
#include "analyzer/hist/V1724Waveform.hxx"

namespace analyzer {

namespace hist {

V1724Waveform::V1724Waveform(VirtualOdb* const anOdb) :
		AbstractWaveform(anOdb, fe::caen::v1724::equipName,
				fe::caen::v1724::displayName,
				caen::v1724::nsPerSample<ns_per_sample_type>()), minFront(14), frontLength(
				3), peakLength(16), threshold(7.0), rising(false) {

}

void V1724Waveform::UpdateHistograms(TDataContainer &dataContainer) {

	using util::caen::V1724InfoRawData;
	using util::TWaveFormRawData;

	auto const v1724Info = dataContainer.GetEventData < V1724InfoRawData
			> (V1724InfoRawData::bankName());
	if (v1724Info) {
		auto const feIndex = frontendIndex(v1724Info->info().frontendIndex);
		auto const edgePosition = FindEdgeDistance(dataContainer, v1724Info);

		for (uint8_t channelNo = 0; channelNo < caen::v1724::NUM_OF_CHANNELS;
				channelNo++) {
			if (v1724Info->channelIncluded(channelNo)) {
				auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
						> (TWaveFormRawData::bankName(channelNo));
				if (wfRaw) {
					auto const numOfSamples = wfRaw->numOfSamples();
					if (numOfSamples > 0) {
						auto const wfBegin = wfRaw->begin();
						auto const wfEnd = wfRaw->end();

						// draw raw waveform
						auto &h = GetWaveformHist(feIndex, channelNo,
								numOfSamples);
						SetData(h, wfBegin, wfEnd);

						AnalyzeWaveform(v1724Info, channelNo, numOfSamples,
								edgePosition, wfBegin, wfEnd);
					}
				}
			}
		}
	}

}

void V1724Waveform::AnalyzeWaveform(
		util::caen::V1724InfoRawData const* const v1724Info,
		uint8_t const channelNo, std::size_t const numOfSamples,
		util::TWaveFormRawData::difference_type const edgePosition,
		util::TWaveFormRawData::const_iterator_type const wfBegin,
		util::TWaveFormRawData::const_iterator_type const wfEnd) {

	auto const wfDiff = math::MakeDiffContainer<int16_t>(wfBegin, wfEnd,
			frontLength);
	auto const diffStat = math::MakeStatAccum(std::begin(wfDiff),
			std::end(wfDiff));
	auto const t = diffStat.GetStdScaled < util::TWaveFormRawData::value_type
			> (threshold);
	auto const hasPeak =
			rising ? diffStat.GetMaxValue() >= t : diffStat.GetMinValue() <= -t;

	if (hasPeak) {
		auto const wfStat = math::MakeStatAccum(wfBegin, wfEnd);
		auto const zeroLevel = wfStat.GetRoughMean();
		auto const feIndex = frontendIndex(v1724Info->info().frontendIndex);
		auto const preTriggerLength =
				v1724Info->hasTriggerSettings() ?
						v1724Info->preTriggerLength() : 0;
		auto &ph = GetPositionHist(feIndex, channelNo, numOfSamples,
				preTriggerLength);
		auto &ah = GetAmplitudeHist(feIndex, channelNo,
				caen::v1724::NUM_OF_SAMPLE_VALUES);

		auto pf = math::MakePeakFinder(rising, wfBegin, wfEnd, frontLength, t,
				peakLength);
		while (pf.HasNext()) {
			auto const i = pf.GetNext();
			auto const position = std::distance(wfBegin, i) - edgePosition
					+ preTriggerLength;
			if (position >= 0) {
				ph.AddBinContent(position);
			}

			decltype(zeroLevel) const ampAdjusted =
					rising ? *i - zeroLevel : zeroLevel - *i;
			auto const amplitude = std::min(ampAdjusted,
					caen::v1724::MAX_SAMPLE_VALUE);
			ah.AddBinContent(amplitude);
		}
	}

}

util::TWaveFormRawData::difference_type V1724Waveform::FindEdgeDistance(
		TDataContainer &dataContainer,
		util::caen::V1724InfoRawData const*v1724Info) {

	using util::TWaveFormRawData;
	using util::TriggerInfoRawData;

	auto const triggerInfo = dataContainer.GetEventData < TriggerInfoRawData
			> (TriggerInfoRawData::bankName());

	if (triggerInfo && v1724Info->hasTriggerSettings()
			&& v1724Info->triggerMode() == 0) {
		auto const triggerChannel = triggerInfo->triggerChannel();

		if (v1724Info->channelIncluded(triggerChannel)) {
			auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
					> (TWaveFormRawData::bankName(triggerChannel));
			if (wfRaw) {
				return math::FindEdgeDistance(triggerInfo->triggerRising(),
						triggerInfo->triggerThreshold(), wfRaw->begin(),
						wfRaw->end());
			}
		}
	}

	return 0;

}

//unsigned V1724Waveform::loadWaveformLength(INT const feIndex) {
//
//	auto const hKeyName = odb::equipSettingsKeyName(getBaseEquipName(), feIndex,
//			fe::v1724::settings::waveformLength);
//
//	if (getOdb()->odbReadArraySize(hKeyName.c_str()) <= 0) {
//		return 0;
//	}
//
//	return getOdb()->odbReadUint32(hKeyName.c_str(), 0, 0);
//
//}
//
//std::vector<bool> V1724Waveform::loadEnabledChannels(INT feIndex) {
//
//	std::vector<bool> result;
//	auto const hKeyName = odb::equipSettingsKeyName(getBaseEquipName(), feIndex,
//			fe::v1724::settings::enabledChannels);
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
