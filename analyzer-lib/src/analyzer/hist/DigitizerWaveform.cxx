#include <analyzer/hist/DigitizerWaveform.hxx>

namespace analyzer {

namespace hist {

DigitizerWaveform::DigitizerWaveform(VirtualOdb* anOdb,
		std::string const& aBaseEquipName, std::string const & aDisplayName,
		ns_per_sample_type const aNsPerSample) :
		AbstractWaveform(anOdb, aBaseEquipName, aDisplayName, aNsPerSample) {
}

void DigitizerWaveform::UpdateHistograms(TDataContainer &dataContainer,
		util::caen::DigitizerInfoRawData const* info) {

	using util::TWaveFormRawData;
	using util::SignalInfoRawData;

	if (info) {
		auto const feIndex = frontendIndex(info->info().frontendIndex);
		auto const signalInfo = dataContainer.GetEventData < SignalInfoRawData
				> (SignalInfoRawData::BANK_NAME);

		DetectTrigger(dataContainer, info);

		for (channel_no_type channelNo = 0; channelNo < numOfChannels();
				channelNo++) {
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

						AnalyzeWaveform(info, channelNo, wfBegin, wfEnd,
								signalInfo ?
										signalInfo->info(channelNo) : nullptr);
					}
				}
			}
		}
	}

}

DigitizerWaveform::channel_no_type DigitizerWaveform::CurrentTrigger(
		util::caen::DigitizerInfoRawData const* const info) const {

	auto const ch = info->firstSelfTriggerChannel();
	if (ch >= 0 && ch < static_cast<int>(numOfChannels())) {
		return ch;
	}

	return EXT_TRIGGER;

}

DigitizerWaveform::channel_no_type DigitizerWaveform::ChannelTrigger(
		util::caen::DigitizerInfoRawData const* const info,
		util::SignalInfoBank const* const signalInfo) const {

	using util::SignalInfoRawData;

	if (signalInfo) {
		auto const ch = SignalInfoRawData::triggerChannel(*signalInfo);
		if (ch >= 0 && ch < static_cast<int>(numOfChannels())) {
			return ch;
		}
	}

	return CurrentTrigger(info);

}

std::pair<bool, util::TWaveFormRawData::value_type> DigitizerWaveform::HasPeaks(
		util::TWaveFormRawData::const_iterator_type const wfBegin,
		util::TWaveFormRawData::const_iterator_type const wfEnd,
		util::SignalInfoBank const* const signalInfo,
		wflength_type const frontLength, bool const rising) const {

	using util::SignalInfoRawData;
	using util::TWaveFormRawData;

	auto const wfDiff = math::MakeDiffContainer<int16_t>(wfBegin, wfEnd,
			frontLength);
	auto const diffStat = math::MakeStatAccum(std::begin(wfDiff),
			std::end(wfDiff));

	// detect signal threshold
	TWaveFormRawData::value_type t;
	if (signalInfo) {
		// a threshold is set for the given channel
		auto const st = SignalInfoRawData::threshold(*signalInfo);
		if (st < 0) {
			// have to calculate threshold using waveform noise level
			t = diffStat.GetStdScaled < TWaveFormRawData::value_type
					> (std::abs(st));
		} else {
			// threshold value is explicitly set
			t = st;
		}
	} else {
		// have to calculate threshold using waveform noise level
		t = diffStat.GetStdScaled < TWaveFormRawData::value_type
				> (DEF_SIGNAL_THRESHOLD_KAPPA);
	}

	auto const hasPeak =
			rising ? diffStat.GetMaxValue() >= t : diffStat.GetMinValue() <= -t;

	return std::pair<bool, util::TWaveFormRawData::value_type>(hasPeak, t);
}

void DigitizerWaveform::AnalyzeWaveform(
		util::caen::DigitizerInfoRawData const* const info,
		channel_no_type const channelNo,
		util::TWaveFormRawData::const_iterator_type const wfBegin,
		util::TWaveFormRawData::const_iterator_type const wfEnd,
		util::SignalInfoBank const* signalInfo) {

	using util::SignalInfoRawData;

	auto const frontLength =
			signalInfo ?
					SignalInfoRawData::frontLength(*signalInfo) :
					DEF_SIGNAL_FRONT_LENGTH;
	auto const rising =
			signalInfo ?
					SignalInfoRawData::rising(*signalInfo) : DEF_SIGNAL_RISING;
	auto const hp = HasPeaks(wfBegin, wfEnd, signalInfo, frontLength, rising);
	auto const hasPeak = hp.first;
	auto const t = hp.second;

	if (hasPeak) {
		auto const peakLength =
				signalInfo ?
						SignalInfoRawData::length(*signalInfo) :
						DEF_SIGNAL_LENGTH;
		auto const triggerChannel = ChannelTrigger(info, signalInfo);
		auto const wfStat = math::MakeStatAccum(wfBegin, wfEnd);
		auto const zeroLevel = wfStat.GetRoughMean();
		auto const feIndex = frontendIndex(info->info().frontendIndex);
		auto const preTriggerLength =
				info->hasTriggerSettings() ? info->preTriggerLength() : 0;
		auto &ph = GetPositionHist(feIndex, channelNo,
				std::distance(wfBegin, wfEnd), preTriggerLength);
		auto &ah = GetAmplitudeHist(feIndex, channelNo, numOfSampleValues());

		auto pf = math::MakePeakFinder(rising, wfBegin, wfEnd, frontLength, t,
				peakLength);
		while (pf.HasNext()) {
			auto const i = pf.GetNext();
			auto const position = std::distance(wfBegin, i)
					- triggers[triggerChannel] /* TODO */
			+ preTriggerLength;
			if (position >= 0) {
				if (position > ph.GetXaxis()->GetXmax()) {
					std::cout << "New hist on position " << position << " ep "
							<< triggers[triggerChannel] << " d "
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

void DigitizerWaveform::DetectTrigger(TDataContainer &dataContainer,
		util::caen::DigitizerInfoRawData const* info) {

	using util::TWaveFormRawData;
	using util::TriggerInfoRawData;

	auto const triggerInfo = dataContainer.GetEventData < TriggerInfoRawData
			> (TriggerInfoRawData::bankName());

	if (triggerInfo && info->hasTriggerSettings()) {
		auto const trigCh = CurrentTrigger(info);
		triggers[trigCh] = 0;
		triggerTimestamps[trigCh] = timeStamp(info->info());

		if (!info->extTrigger() && info->channelIncluded(trigCh)) {
			auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
					> (TWaveFormRawData::bankName(trigCh));
			if (wfRaw) {
				auto const ti = triggerInfo->channelInfo(trigCh);
				if (ti) {
					triggers[trigCh] = math::FindEdgeDistance(
							TriggerInfoRawData::rising(*ti),
							TriggerInfoRawData::threshold(*ti), wfRaw->begin(),
							wfRaw->end());
				}
			}
		}

	}

}

}

}
