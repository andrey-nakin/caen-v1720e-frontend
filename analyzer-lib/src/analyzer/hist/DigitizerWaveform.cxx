#include <analyzer/hist/DigitizerWaveform.hxx>

namespace analyzer {

namespace hist {

DigitizerWaveform::DigitizerWaveform(VirtualOdb* anOdb,
		std::string const& aBaseEquipName, std::string const & aDisplayName,
		ns_per_sample_type const aNsPerSample) :
		AbstractWaveform(anOdb, aBaseEquipName, aDisplayName, aNsPerSample), minFront(
				14), frontLength(3), peakLength(16), threshold(7.0), rising(
				false), masterEventOccurred(false), lastMasterEdgeDistance(0) {
}

void DigitizerWaveform::UpdateHistograms(TDataContainer &dataContainer,
		util::caen::DigitizerInfoRawData const* info) {

	using util::TWaveFormRawData;
	using util::SignalInfoRawData;

	if (info) {
		auto const feIndex = frontendIndex(info->info().frontendIndex);
		auto const edgePosition = FindEdgeDistance(dataContainer, info);
		auto const signalInfo = dataContainer.GetEventData < SignalInfoRawData
				> (SignalInfoRawData::BANK_NAME);

		for (uint8_t channelNo = 0; channelNo < numOfChannels(); channelNo++) {
			if (info->channelIncluded(channelNo)) {
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

						AnalyzeWaveform(info, channelNo, numOfSamples,
								edgePosition, wfBegin, wfEnd,
								signalInfo ?
										signalInfo->info(channelNo) : nullptr);
					}
				}
			}
		}
	}

}

void DigitizerWaveform::AnalyzeWaveform(
		util::caen::DigitizerInfoRawData const* const info,
		uint8_t const channelNo, std::size_t const numOfSamples,
		util::TWaveFormRawData::difference_type const edgePosition,
		util::TWaveFormRawData::const_iterator_type const wfBegin,
		util::TWaveFormRawData::const_iterator_type const wfEnd,
		util::SignalInfoBank const* signalInfo) {

	using util::SignalInfoRawData;

	auto const peakLength =
			signalInfo ?
					SignalInfoRawData::length(signalInfo) : this->peakLength;
	auto const frontLength =
			signalInfo ?
					SignalInfoRawData::frontLength(signalInfo) :
					this->frontLength;
	auto const rising =
			signalInfo ? SignalInfoRawData::rising(signalInfo) : this->rising;

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
		auto const feIndex = frontendIndex(info->info().frontendIndex);
		auto const preTriggerLength =
				info->hasTriggerSettings() ? info->preTriggerLength() : 0;
		auto &ph = GetPositionHist(feIndex, channelNo, numOfSamples,
				preTriggerLength);
		auto &ah = GetAmplitudeHist(feIndex, channelNo, numOfSampleValues());

		auto pf = math::MakePeakFinder(rising, wfBegin, wfEnd, frontLength, t,
				peakLength);
		while (pf.HasNext()) {
			auto const i = pf.GetNext();
			auto const position = std::distance(wfBegin, i) - edgePosition
					+ preTriggerLength;
			if (position >= 0) {
				if (position > ph.GetXaxis()->GetXmax()) {
					std::cout << "New hist on position " << position << " ep "
							<< edgePosition << " d "
							<< std::distance(wfBegin, i) << std::endl; //	TODO
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

util::TWaveFormRawData::difference_type DigitizerWaveform::FindEdgeDistance(
		TDataContainer &dataContainer,
		util::caen::DigitizerInfoRawData const* info) {

	using util::TWaveFormRawData;
	using util::TriggerInfoRawData;

	auto const triggerInfo = dataContainer.GetEventData < TriggerInfoRawData
			> (TriggerInfoRawData::bankName());

	if (triggerInfo && info->hasTriggerSettings() && info->triggerMode() == 0) {

		auto const master = triggerInfo->masterTriggerChannel();
		if (master < 0) {
			// no master trigger
			auto const trigger = info->firstSelfTriggerChannel();
			if (trigger < 0) {
				// this event is not caused by any self-trigger channel
			} else {
				if (info->channelIncluded(trigger)) {
					auto const wfRaw = dataContainer.GetEventData
							< TWaveFormRawData
							> (TWaveFormRawData::bankName(trigger));
					if (wfRaw) {
						auto const idx = triggerInfo->triggerChannelIndex(
								trigger);
						if (idx >= 0) {
							auto const e = math::FindEdgeDistance(
									triggerInfo->triggerRising(idx),
									triggerInfo->triggerThreshold(idx),
									wfRaw->begin(), wfRaw->end());
							if (true || e < 500) {
								std::cout << "Edge found at " << e << std::endl;
							}
							return math::FindEdgeDistance(
									triggerInfo->triggerRising(idx),
									triggerInfo->triggerThreshold(idx),
									wfRaw->begin(), wfRaw->end());
						}
					}
				}
			}
		} else {
			// use master trigger
			if (info->selfTrigger(master)) {
				// this event is caused by master trigger
				if (info->channelIncluded(master)) {
					auto const wfRaw = dataContainer.GetEventData
							< TWaveFormRawData
							> (TWaveFormRawData::bankName(master));
					if (wfRaw) {
						auto const idx = triggerInfo->triggerChannelIndex(
								master);
						if (idx >= 0) {
							masterEventOccurred = true;
							lastMasterEvent = info->info();
							lastMasterEdgeDistance = math::FindEdgeDistance(
									triggerInfo->triggerRising(idx),
									triggerInfo->triggerThreshold(idx),
									wfRaw->begin(), wfRaw->end());
							return lastMasterEdgeDistance;
						}
					}
				}
			} else {
				// this event is caused by non-master trigger
				if (masterEventOccurred) {
					auto const tm = samplesPerTimeTick()
							* timeDiff(timeStamp(lastMasterEvent),
									timeStamp(info->info()));
					if (tm < 100000) {
						return lastMasterEdgeDistance - tm;
					}
				}
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
