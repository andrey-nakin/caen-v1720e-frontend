#ifndef AN_BASIC_WAVE_FORM_HIST_V1720Waveform_hxx
#define AN_BASIC_WAVE_FORM_HIST_V1720Waveform_hxx

#include "AbstractWaveform.hxx"

namespace bwf {

namespace hist {

class V1720Waveform: public AbstractWaveform {
public:

	V1720Waveform();
	~V1720Waveform() override;

	void UpdateHistograms(TDataContainer &dataContainer) override;

private:

	void createHistograms();

};

}

}

#endif   //	AN_BASIC_WAVE_FORM_HIST_V1720Waveform_hxx
