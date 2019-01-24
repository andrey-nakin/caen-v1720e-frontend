#include <iomanip>
#include <sstream>
#include "analyzer/hist/AbstractWaveform.hxx"

namespace analyzer {

namespace hist {

AbstractWaveform::AbstractWaveform(VirtualOdb* anOdb,
		std::string const& aBaseEquipName, std::string const & aDisplayName,
		ns_per_sample_type const aNsPerSample) :
		odb(anOdb), baseEquipName(aBaseEquipName), displayName(aDisplayName), nsPerSample(
				aNsPerSample) {

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
		unsigned const channelNo, unsigned const waveformLength) {

	return FindCreatePositionHist(feIndex, channelNo, waveformLength);

}

AbstractWaveform::HistType& AbstractWaveform::GetAmplitudeHist(
		INT const feIndex, unsigned const channelNo, unsigned const maxValue) {

	return FindCreateAmplitudeHist(feIndex, channelNo, maxValue);

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
		unsigned const waveformLength) {

	auto const name = ConstructName(feIndex, channelNo, "PO");
	auto const title = ConstructTitle(feIndex, channelNo, "Position");

	auto const h = new HistType(name.c_str(), title.c_str(), waveformLength, 0,
			waveformLength);
	h->SetXTitle("Channel");
	h->SetYTitle("Count");
	push_back(h);

	return h;

}

AbstractWaveform::HistType* AbstractWaveform::CreateAmplitudeHistogram(
		INT const feIndex, unsigned const channelNo, unsigned const maxValue) {

	auto const name = ConstructName(feIndex, channelNo, "AM");
	auto const title = ConstructTitle(feIndex, channelNo, "Amplitude");

	auto const h = new HistType(name.c_str(), title.c_str(), maxValue, 0,
			maxValue);
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
		unsigned const waveformLength) {

	auto const& m = posHistograms[feIndex];
	auto const& j = m.find(channelNo);
	if (m.end() != j) {
		return *j->second;
	} else {
		auto const h = CreatePositionHistogram(feIndex, channelNo,
				waveformLength);
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
