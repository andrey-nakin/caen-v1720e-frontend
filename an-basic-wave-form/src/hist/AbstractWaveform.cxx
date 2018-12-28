#include <iomanip>
#include <sstream>
#include "hist/AbstractWaveform.hxx"

namespace bwf {

namespace hist {

AbstractWaveform::AbstractWaveform(VirtualOdb* anOdb,
		std::string const& aBaseEquipName, std::string const & aDisplayName,
		ns_per_sample_type const aNsPerSample) :
		odb(anOdb), baseEquipName(aBaseEquipName), displayName(aDisplayName), nsPerSample(
				aNsPerSample) {

}

AbstractWaveform::~AbstractWaveform() {

}

void AbstractWaveform::UpdateHistograms(TDataContainer &/*dataContainer*/) {

}

void AbstractWaveform::CreateHistograms() {

	createHistograms(-1);
	for (INT idx = 0; idx < 100; idx++) {
		createHistograms(idx);
	}

}

void AbstractWaveform::createHistograms(INT const feIndex) {
	auto const waveformLength = loadWaveformLength(feIndex);
	if (waveformLength == 0) {
		// no such equipment or wave form length is not specified
		return;
	}

	auto const enabledChannels = loadEnabledChannels(feIndex);
	if (enabledChannels.empty()) {
		return;
	}

	for (unsigned channelNo = 0; channelNo < enabledChannels.size();
			channelNo++) {

		if (!enabledChannels[channelNo]) {
			continue;
		}

		auto const name = ConstructName(feIndex, channelNo);
		auto const title = ConstructTitle(feIndex, channelNo);

		auto const h = new TH1D(name.c_str(), title.c_str(), waveformLength, 0,
				waveformLength * nsPerSample);
		h->SetXTitle("Time, ns");
		h->SetYTitle("ADC Value");
		push_back(h);

		for (int i = 0; i < waveformLength; i++) {
			h->SetBinContent(i + 1, rand() % 4096);
		}
	}
}

std::string AbstractWaveform::ConstructName(INT const feIndex,
		unsigned const channelNo) {

	std::stringstream s;
	s << displayName;
	if (feIndex >= 0) {
		s << '_' << std::setfill('0') << std::setw(2) << feIndex;
	}
	s << '_' << channelNo;
	return s.str();

}

std::string AbstractWaveform::ConstructTitle(INT const feIndex,
		unsigned const channelNo) {

	std::stringstream s;
	s << displayName;
	if (feIndex >= 0) {
		s << " #" << feIndex;
	}
	s << ", Channel #" << channelNo << ", Waveform";
	return s.str();

}

}

}
