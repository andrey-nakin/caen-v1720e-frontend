#include <iomanip>
#include <sstream>
#include <analyzer/hist/AbstractWaveform.hxx>
#include <analyzer/util/AnalyzerUtils.hxx>

namespace analyzer {

namespace hist {

template<typename BinsT, typename MaxBinsT>
constexpr BinsT numOfNums(BinsT const bins, MaxBinsT const maxBins) {

	return maxBins == 0 || maxBins > bins ? bins : maxBins;

}

AbstractWaveform::AbstractWaveform(VirtualOdb* anOdb,
		std::string const& anOdbRootKey, std::string const& aBaseEquipName,
		std::string const & aDisplayName, ns_per_sample_type const aNsPerSample) :
		odb(anOdb), odbRootKey(anOdbRootKey), baseEquipName(aBaseEquipName), displayName(
				aDisplayName), nsPerSample(aNsPerSample) {

}

//void AbstractWaveform::CreateHistograms() {
//
//	createHistograms(-1);
//	for (INT idx = 0; idx < 100; idx++) {
//		createHistograms(idx);
//	}
//
//}

AbstractWaveform::HistType& AbstractWaveform::GetWaveformHist(INT const feIndex,
		unsigned const channelNo, unsigned const waveformLength) {

	auto& h = FindCreateWaveformHist(feIndex, channelNo, waveformLength);
	if (!histInitialized[feIndex][channelNo]) {
		ResetHistogram(h, waveformLength);
		histInitialized[feIndex][channelNo] = true;
	}
	return h;

}

AbstractWaveform::HistType& AbstractWaveform::GetPositionHist(INT const feIndex,
		unsigned const channelNo, unsigned const waveformLength,
		unsigned const preTriggerLength) {

	return FindCreatePositionHist(feIndex, channelNo, waveformLength,
			preTriggerLength);

}

AbstractWaveform::HistType& AbstractWaveform::RebinPositionHist(
		INT const feIndex, unsigned const channelNo,
		unsigned const waveformLength, unsigned const preTriggerLength) {

	auto& h = GetPositionHist(feIndex, channelNo, waveformLength,
			preTriggerLength);
	h.SetBins(waveformLength + preTriggerLength,
			-static_cast<double>(preTriggerLength), waveformLength);
	return h;

}

AbstractWaveform::HistType& AbstractWaveform::GetAmplitudeHist(
		INT const feIndex, unsigned const channelNo, unsigned const maxValue) {

	return FindCreateAmplitudeHist(feIndex, channelNo, maxValue);

}

void AbstractWaveform::FillPositionHist(HistType& ph, unsigned const position,
		unsigned const preTriggerLength) {

	if (position > ph.GetXaxis()->GetXmax()) {
		auto const bins = position + preTriggerLength;
		ph.SetBins(numOfNums(bins, GetPositionMaxBins()),
				-static_cast<double>(preTriggerLength), position);
	}
	ph.Fill(position);

}

//void AbstractWaveform::createHistograms(INT const feIndex) {
//	auto const waveformLength = loadWaveformLength(feIndex);
//	if (waveformLength == 0) {
//		// no such equipment or wave form length is not specified
//		return;
//	}
//
//	auto const enabledChannels = loadEnabledChannels(feIndex);
//	if (enabledChannels.empty()) {
//		return;
//	}
//
//	histograms.clear();
//
//	for (unsigned channelNo = 0; channelNo < enabledChannels.size();
//			channelNo++) {
//
//		if (!enabledChannels[channelNo]) {
//			continue;
//		}
//
//		histograms[feIndex][channelNo] = CreateHistogram(feIndex, channelNo, waveformLength);
//	}
//}

void AbstractWaveform::BeginRun(int /*transition*/, int /*run*/, int /*time*/) {

	histInitialized.clear();

}

unsigned AbstractWaveform::GetPositionMaxBins() const {

	return odb->odbReadUint32(
			util::AnalyzerUtils::OdbKey(getOdbRootKey(), "time_hist_max_bins").c_str(),
			0, 0);

}

unsigned AbstractWaveform::GetAmplitudeMaxBins() const {

	return odb->odbReadUint32(
			util::AnalyzerUtils::OdbKey(getOdbRootKey(), "amp_hist_max_bins").c_str(),
			0, 0);

}

AbstractWaveform::HistType* AbstractWaveform::CreateWaveformHistogram(
		INT const feIndex, unsigned const channelNo,
		unsigned const waveformLength) {

	auto const name = ConstructName(feIndex, channelNo, "WF");
	auto const title = ConstructTitle(feIndex, channelNo, "Waveform");

	auto const h = new HistType(name.c_str(), title.c_str(), waveformLength, 0,
			waveformLength * nsPerSample);
	h->SetXTitle("Time, ns");
	h->SetYTitle("ADC Value");
	h->SetStats(kFALSE);
	push_back(h);

	return h;

}

AbstractWaveform::HistType* AbstractWaveform::CreatePositionHistogram(
		INT const feIndex, unsigned const channelNo,
		unsigned const waveformLength, unsigned const preTriggerLength) {

	auto const name = ConstructName(feIndex, channelNo, "TM");
	auto const title = ConstructTitle(feIndex, channelNo, "Time");

	auto const bins = waveformLength + preTriggerLength;
	auto const h = new HistType(name.c_str(), title.c_str(),
			numOfNums(bins, GetPositionMaxBins()),
			-static_cast<double>(preTriggerLength), waveformLength);
	h->SetXTitle("Channel");
	h->SetYTitle("Count");
	h->SetCanExtend(HistType::kAllAxes);
	push_back(h);

	return h;

}

AbstractWaveform::HistType* AbstractWaveform::CreateAmplitudeHistogram(
		INT const feIndex, unsigned const channelNo, unsigned const maxValue) {

	auto const name = ConstructName(feIndex, channelNo, "AM");
	auto const title = ConstructTitle(feIndex, channelNo, "Amplitude");

	auto const bins = maxValue;
	auto const h = new HistType(name.c_str(), title.c_str(),
			numOfNums(bins, GetAmplitudeMaxBins()), 0, maxValue);
	h->SetXTitle("Amplitude");
	h->SetYTitle("Count");
	push_back(h);

	return h;

}

AbstractWaveform::HistType& AbstractWaveform::FindCreateWaveformHist(
		INT const feIndex, unsigned const channelNo,
		unsigned const waveformLength) {

	auto const& m = wfHistograms[feIndex];
	auto const& j = m.find(channelNo);
	if (m.end() != j) {
		return *j->second;
	} else {
		auto const h = CreateWaveformHistogram(feIndex, channelNo,
				waveformLength);
		wfHistograms[feIndex][channelNo] = h;
		return *h;
	}

}

AbstractWaveform::HistType& AbstractWaveform::FindCreatePositionHist(
		INT const feIndex, unsigned const channelNo,
		unsigned const waveformLength, unsigned const preTriggerLength) {

	auto const& m = posHistograms[feIndex];
	auto const& j = m.find(channelNo);
	if (m.end() != j) {
		return *j->second;
	} else {
		auto const h = CreatePositionHistogram(feIndex, channelNo,
				waveformLength, preTriggerLength);
		posHistograms[feIndex][channelNo] = h;
		return *h;
	}

}

AbstractWaveform::HistType& AbstractWaveform::FindCreateAmplitudeHist(
		INT const feIndex, unsigned const channelNo, unsigned const maxValue) {

	auto const& m = ampHistograms[feIndex];
	auto const& j = m.find(channelNo);
	if (m.end() != j) {
		return *j->second;
	} else {
		auto const h = CreateAmplitudeHistogram(feIndex, channelNo, maxValue);
		ampHistograms[feIndex][channelNo] = h;
		return *h;
	}

}

void AbstractWaveform::ResetHistogram(HistType& h,
		unsigned const waveformLength) {

	h.SetBins(waveformLength, 0, waveformLength * nsPerSample);

}

std::string AbstractWaveform::ConstructName(INT const feIndex,
		unsigned const channelNo, const char* const name) {

	std::stringstream s;
	s << name << '_' << displayName;
	if (feIndex >= 0) {
		s << std::setfill('0') << std::setw(2) << feIndex;
	}
	s << '_' << channelNo;
	return s.str();

}

std::string AbstractWaveform::ConstructTitle(INT const feIndex,
		unsigned const channelNo, const char* const title) {

	std::stringstream s;
	s << title << ' ' << displayName;
	if (feIndex >= 0) {
		s << " #" << feIndex;
	}
	s << ", Channel #" << channelNo;
	return s.str();

}

}

}
