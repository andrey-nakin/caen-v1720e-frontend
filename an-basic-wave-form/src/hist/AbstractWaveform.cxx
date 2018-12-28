#include "hist/AbstractWaveform.hxx"

namespace bwf {

namespace hist {

AbstractWaveform::AbstractWaveform(VirtualOdb* anOdb, std::string const& aBaseEquipName,
		std::string const & aDisplayName,
		ns_per_sample_type const aNsPerSample) :
		odb(anOdb), baseEquipName(aBaseEquipName), displayName(
				aDisplayName), nsPerSample(aNsPerSample) {

	createHistograms();

}

void AbstractWaveform::UpdateHistograms(TDataContainer &/*dataContainer*/) {

}

void AbstractWaveform::createHistograms() {

	createHistograms(-1);
	for (INT idx = 0; idx < 2; idx++) {	//	@TODO
		createHistograms(idx);
	}

	unsigned channelNo = 0;
	unsigned const recordLen = 1024;

	std::stringstream sName;
	sName << displayName << '_' << channelNo;
	auto const name = sName.str();

	std::stringstream sTitle;
	sTitle << displayName << " Waveform, Channel #" << channelNo;
	auto const title = sTitle.str();

	auto const h = new TH1D(name.c_str(), title.c_str(), recordLen, 0,
			recordLen * nsPerSample);
	h->SetXTitle("Time, ns");
	h->SetYTitle("ADC Value");
	push_back(h);

}

void AbstractWaveform::createHistograms(INT const feIndex) {
	std::cout << "FE " << feIndex << " checking " << std::endl;

	int waveformLength = loadWaveformLength(feIndex);
	if (waveformLength < 0) {
		// no such equipment or wave form length is not specified
		return;
	}

	std::cout << "FE " << feIndex << ", waveformLength = " << waveformLength
			<< std::endl;
}

}

}
