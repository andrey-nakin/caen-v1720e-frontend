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
public:

	AbstractWaveform(std::string const& aBaseEquipName,
			std::string const & aDisplayName, unsigned const aNsInSample) :
			baseEquipName(aBaseEquipName), displayName(aDisplayName), nsInSample(
					aNsInSample) {

		createHistograms();

	}

	void UpdateHistograms(TDataContainer &/*dataContainer*/) override {

	}

private:

	std::string const baseEquipName;
	std::string const displayName;
	unsigned const nsInSample;

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
				recordLen * nsInSample);
		h->SetXTitle("Time, ns");
		h->SetYTitle("ADC Value");
		push_back(h);

	}

};

}

}

#endif   //	AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx
