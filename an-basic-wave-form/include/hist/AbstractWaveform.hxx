#ifndef AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx
#define AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx

#include <string>
#include <sstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"

#include <THistogramArrayBase.h>

#pragma GCC diagnostic pop

namespace bwf {

namespace hist {

template<typename SampleType>
class AbstractWaveform: public THistogramArrayBase {
protected:

	typedef unsigned ns_per_sample_type;

public:

	AbstractWaveform(std::string const& aBaseEquipName,
			std::string const & aDisplayName, ns_per_sample_type const aNsPerSample) :
			baseEquipName(aBaseEquipName), displayName(aDisplayName), nsPerSample(
					aNsPerSample) {

		createHistograms();

	}

	void UpdateHistograms(TDataContainer &/*dataContainer*/) override {

	}

private:

	std::string const baseEquipName;
	std::string const displayName;
	ns_per_sample_type const nsPerSample;

	void createHistograms() {

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

};

}

}

#endif   //	AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx
