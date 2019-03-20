#include <analyzer/hist/DigitizerWaveform.hxx>

namespace analyzer {

namespace hist {

DigitizerWaveform::DigitizerWaveform(VirtualOdb* anOdb,
		std::string const& anOdbRootKey, std::string const& aBaseEquipName,
		std::string const & aDisplayName, ns_per_sample_type const aNsPerSample) :
		AbstractWaveform(anOdb, anOdbRootKey, aBaseEquipName, aDisplayName,
				aNsPerSample) {
}

void DigitizerWaveform::UpdateHistograms(TDataContainer &dataContainer,
		util::caen::DigitizerInfoRawData const& info) {

	using util::TWaveFormRawData;
	using util::SignalInfoRawData;

	auto const feIndex = frontendIndex(info.info().frontendIndex);
	auto const signalInfo = dataContainer.GetEventData < SignalInfoRawData
			> (SignalInfoRawData::BANK_NAME);

	DetectTrigger(dataContainer, info);

	for (channel_no_type channelNo = 0; channelNo < numOfChannels();
			channelNo++) {
		if (info.channelIncluded(channelNo)) {
			auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
					> (TWaveFormRawData::bankName(channelNo));
			if (wfRaw) {
				auto const numOfSamples = wfRaw->numOfSamples();
				if (numOfSamples > 0) {
					auto const wfBegin = wfRaw->begin();
					auto const wfEnd = wfRaw->end();

					// draw raw waveform
					auto &h = GetWaveformHist(feIndex, channelNo, numOfSamples);
					SetData(h, wfBegin, wfEnd);

					AnalyzeWaveform(info, channelNo, wfBegin, wfEnd,
							signalInfo ? signalInfo->info(channelNo) : nullptr);
				}
			}
		}
	}

}

DigitizerWaveform::channel_no_type DigitizerWaveform::CurrentTrigger(
		util::caen::DigitizerInfoRawData const& info) const {

	auto const chMask = info.selfTriggerChannels();
	if (chMask != 0) {
		for (channel_type ch = 0, last = numOfChannels(); ch < last; ch++) {
			if (info.selfTrigger(ch)) {
				return ch;
			}
		}
	}

	return EXT_TRIGGER;

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
		if (st == 0) {
			// use trigger settings
			return std::make_pair(true, 0);
		} else if (st < 0) {
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

	return std::make_pair(hasPeak, t);
}

std::pair<bool, DigitizerWaveform::distance_type> DigitizerWaveform::CalcPosition(
		util::caen::DigitizerInfoRawData const& info, distance_type const wfPos,
		util::SignalInfoBank const* const signalInfo) {

	using util::SignalInfoRawData;

	auto result = wfPos;

	auto const curTrg = CurrentTrigger(info);
	if (curTrg != EXT_TRIGGER && triggers.count(curTrg) > 0) {
		result -= triggers[curTrg];
	}

	if (signalInfo && SignalInfoRawData::timeTriggers(*signalInfo) != 0) {
		distance_type tm = 0;

		for (channel_type ch = 0, last = numOfChannels(); ch < last; ch++) {
			if (SignalInfoRawData::timeTrigger(*signalInfo, ch)) {
				if (triggers.count(ch) > 0) {
					auto const dist = timestampDiff(triggerTimestamps[ch],
							timestamp(info)) * samplesPerTimeTick()
							- triggers[ch];
					if (!tm || tm > dist) {
						tm = dist;
					}
				}
			}
		}

		result += tm;
	}

	auto const maxTime = signalInfo ? signalInfo->maxTime : MAX_POSITION;
	if (result >= maxTime) {
		return std::make_pair(false, 0);	//	discard the position
	}

	return std::make_pair(true, result);

}

void DigitizerWaveform::AnalyzeWaveform(
		util::caen::DigitizerInfoRawData const& info,
		channel_no_type const channelNo,
		util::TWaveFormRawData::const_iterator_type const wfBegin,
		util::TWaveFormRawData::const_iterator_type const wfEnd,
		util::SignalInfoBank const* signalInfo) {

	using util::SignalInfoRawData;

	if (signalInfo
			&& SignalInfoRawData::triggerDisabled(*signalInfo,
					CurrentTrigger(info))) {
		return;
	}

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
		auto const feIndex = frontendIndex(info.info().frontendIndex);
		auto const preTriggerLength =
				info.hasTriggerSettings() ? info.preTriggerLength() : 0;
		auto &ph = GetPositionHist(feIndex, channelNo,
				std::distance(wfBegin, wfEnd), preTriggerLength);
		auto &ah = GetAmplitudeHist(feIndex, channelNo, numOfSampleValues());

		if (t == 0) {
			// do not find spikes, just use current trigger's settings
			if (channelNo == CurrentTrigger(info)) {
				auto const triggerPosition = wfBegin + triggers[channelNo];
				auto const last = std::min(triggerPosition + peakLength, wfEnd);
				auto const peak =
						rising ?
								std::max_element(triggerPosition, last) :
								std::min_element(triggerPosition, last);
				if (peak != last) {
					AddTimeAmplitude(info, wfBegin, wfEnd, ph, ah, signalInfo,
							preTriggerLength, peakLength, rising, peak);
				}
			}
		} else {
			auto pf = math::MakePeakFinder(rising, wfBegin, wfEnd, frontLength,
					t, peakLength);
			while (pf.HasNext()) {
				AddTimeAmplitude(info, wfBegin, wfEnd, ph, ah, signalInfo,
						preTriggerLength, peakLength, rising, pf.GetNext());
			}
		}
	}

}

void DigitizerWaveform::AddTimeAmplitude(
		::util::caen::DigitizerInfoRawData const& info,
		::util::TWaveFormRawData::const_iterator_type const wfBegin,
		::util::TWaveFormRawData::const_iterator_type const wfEnd, HistType& ph,
		HistType& ah, ::util::SignalInfoBank const* signalInfo,
		uint32_t const preTriggerLength, uint32_t const peakLength,
		bool const rising,
		::util::TWaveFormRawData::const_iterator_type const i) {

	auto const position = CalcPosition(info, std::distance(wfBegin, i),
			signalInfo);
	if (position.first) {
		FillPositionHist(ph, position.second, preTriggerLength);

		auto const zeroLevel = math::MakeStatAccum(wfBegin, wfEnd, i,
				i + peakLength).GetRoughMean();
		decltype(zeroLevel) const ampAdjusted =
				rising ? *i - zeroLevel : zeroLevel - *i;
		auto const amplitude = std::min(ampAdjusted, maxSampleValue());
		ah.Fill(amplitude);
	}

}

void DigitizerWaveform::DetectTrigger(TDataContainer &dataContainer,
		util::caen::DigitizerInfoRawData const& info) {

	using util::TWaveFormRawData;
	using util::TriggerInfoRawData;

	auto const trigCh = CurrentTrigger(info);
	triggers[trigCh] = 0;
	triggerTimestamps[trigCh] = timestamp(info);

	if (!info.extTrigger() && info.channelIncluded(trigCh)) {
		auto const triggerInfo = dataContainer.GetEventData < TriggerInfoRawData
				> (TriggerInfoRawData::bankName());
		auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
				> (TWaveFormRawData::bankName(trigCh));
		if (triggerInfo && wfRaw) {
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
