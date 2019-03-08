#ifndef ANALYZER_HIST_V1720Waveform_hxx
#define ANALYZER_HIST_V1720Waveform_hxx

#include <util/caen/V1720InfoRawData.hxx>
#include "DigitizerWaveform.hxx"

namespace analyzer {

namespace hist {

class V1720Waveform: public DigitizerWaveform {
public:

	V1720Waveform(VirtualOdb* anOdb);

	void UpdateHistograms(TDataContainer &dataContainer) override;

private:

	unsigned samplesPerTimeTick() const override;
	unsigned numOfChannels() const override;
	unsigned numOfSampleValues() const override;
	uint16_t maxSampleValue() const override;

};

}

}

#endif   //	ANALYZER_HIST_V1720Waveform_hxx
