#pragma once

#include "AbstractWaveform.hxx"
#include <util/TWaveFormRawData.hxx>
#include <util/TriggerInfoRawData.hxx>
#include <math/EdgeFinder.hxx>
#include <math/DiffContainer.hxx>
#include <math/StatAccum.hxx>
#include <math/PeakFinder.hxx>

namespace analyzer {

namespace hist {

class DigitizerWaveform: public AbstractWaveform {
protected:

	DigitizerWaveform(VirtualOdb* anOdb, std::string const& aBaseEquipName,
			std::string const & aDisplayName,
			ns_per_sample_type const aNsPerSample);

	template<typename InfoRawData>
	void UpdateHistograms(TDataContainer &dataContainer,
			InfoRawData const* info) {

		using util::TWaveFormRawData;

		if (info) {
			auto const feIndex = frontendIndex(info->info().frontendIndex);
			auto const edgePosition = FindEdgeDistance(dataContainer, info);

			for (uint8_t channelNo = 0; channelNo < numOfChannels();
					channelNo++) {
				if (info->channelIncluded(channelNo)) {
					auto const wfRaw = dataContainer.GetEventData
							< TWaveFormRawData
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

							AnalyzeWaveform(info, channelNo, numOfSamples,
									edgePosition, wfBegin, wfEnd);
						}
					}
				}
			}
		}

	}

	template<typename InfoRawData>
	void AnalyzeWaveform(InfoRawData const* const info, uint8_t const channelNo,
			std::size_t const numOfSamples,
			util::TWaveFormRawData::difference_type const edgePosition,
			util::TWaveFormRawData::const_iterator_type const wfBegin,
			util::TWaveFormRawData::const_iterator_type const wfEnd) {

		auto const wfDiff = math::MakeDiffContainer<int16_t>(wfBegin, wfEnd,
				frontLength);
		auto const diffStat = math::MakeStatAccum(std::begin(wfDiff),
				std::end(wfDiff));
		auto const t = diffStat.GetStdScaled
				< util::TWaveFormRawData::value_type > (threshold);
		auto const hasPeak =
				rising ?
						diffStat.GetMaxValue() >= t :
						diffStat.GetMinValue() <= -t;

		if (hasPeak) {
			auto const wfStat = math::MakeStatAccum(wfBegin, wfEnd);
			auto const zeroLevel = wfStat.GetRoughMean();
			auto const feIndex = frontendIndex(info->info().frontendIndex);
			auto const preTriggerLength =
					info->hasTriggerSettings() ? info->preTriggerLength() : 0;
			auto &ph = GetPositionHist(feIndex, channelNo, numOfSamples,
					preTriggerLength);
			auto &ah = GetAmplitudeHist(feIndex, channelNo,
					numOfSampleValues());

			auto pf = math::MakePeakFinder(rising, wfBegin, wfEnd, frontLength,
					t, peakLength);
			while (pf.HasNext()) {
				auto const i = pf.GetNext();
				auto const position = std::distance(wfBegin, i) - edgePosition
						+ preTriggerLength;
				if (position >= 0) {
					if (position > ph.GetXaxis()->GetXmax()) {
						std::cout << "New hist on position " << position
								<< std::endl;	//	TODO
//						auto& phNew = RebinPositionHist(feIndex, channelNo,
//								position, preTriggerLength);
//						phNew.AddBinContent(position);
//					} else {
//						ph.AddBinContent(position);
					}
					ph.Fill(position);
				}

				decltype(zeroLevel) const ampAdjusted =
						rising ? *i - zeroLevel : zeroLevel - *i;
				auto const amplitude = std::min(ampAdjusted, maxSampleValue());
				ah.Fill(amplitude);
			}
		}

	}

	template<typename InfoRawData>
	util::TWaveFormRawData::difference_type FindEdgeDistance(
			TDataContainer &dataContainer, InfoRawData const* info) {

		using util::TWaveFormRawData;
		using util::TriggerInfoRawData;

		auto const triggerInfo = dataContainer.GetEventData < TriggerInfoRawData
				> (TriggerInfoRawData::bankName());

		if (triggerInfo && info->hasTriggerSettings()
				&& info->triggerMode() == 0) {

			auto const master = triggerInfo->masterTriggerChannel();
			if (master < 0) {
				// no master trigger
				// TODO
			} else {
				// use master trigger
				if (info->selfTrigger(master)) {
					// this event is caused by master trigger
					if (info->channelIncluded(master)) {
						auto const wfRaw = dataContainer.GetEventData
								< TWaveFormRawData
								> (TWaveFormRawData::bankName(master));
						if (wfRaw) {
							masterEventOccurred = true;
							lastMasterEvent = info->info();
							lastMasterEdgeDistance = math::FindEdgeDistance(
									triggerInfo->triggerRising(master),
									triggerInfo->triggerThreshold(master),
									wfRaw->begin(), wfRaw->end());
							return lastMasterEdgeDistance;
						}
					}
				} else {
					// this event is caused by non-master trigger
					if (masterEventOccurred) {
						std::cout << "Non-master " << timeStamp(lastMasterEvent)
								<< "\t" << timeStamp(info->info())
								<< (timeStamp(info->info())
										- timeStamp(lastMasterEvent))
								<< std::endl;				//	TODO
						auto const tm = samplesPerTimeTick()
								* timeDiff(timeStamp(lastMasterEvent),
										timeStamp(info->info()));
						if (tm < 10000000) {
							return lastMasterEdgeDistance - tm;
						}
					}
				}
			}

//			auto const triggerChannel = triggerInfo->triggerChannel();
//
//			if (info->channelIncluded(triggerChannel)) {
//				auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
//						> (TWaveFormRawData::bankName(triggerChannel));
//				if (wfRaw) {
//					return math::FindEdgeDistance(triggerInfo->triggerRising(),
//							triggerInfo->triggerThreshold(), wfRaw->begin(),
//							wfRaw->end());
//				}
//			}
		}

		return 0;

	}

private:

	uint16_t minFront;
	unsigned frontLength, peakLength;
	double threshold;
	bool rising;
	bool masterEventOccurred;
	util::TWaveFormRawData::difference_type lastMasterEdgeDistance;
	util::InfoBank lastMasterEvent;

	virtual unsigned samplesPerTimeTick() const = 0;
	virtual unsigned numOfChannels() const = 0;
	virtual unsigned numOfSampleValues() const = 0;
	virtual uint16_t maxSampleValue() const = 0;

	static uint32_t timeStamp(util::InfoBank const& info) {

		return info.timeStamp & ~0x80000000;

	}

	static uint32_t timeDiff(uint32_t const first, uint32_t const last) {

		return (last - first) % static_cast<uint32_t>(0x80000000);

	}

	template<typename IntT>
	static INT frontendIndex(IntT const v) {

		return v == static_cast<decltype(v)>(-1) ? -1 : static_cast<INT>(v);

	}

};

}

}
