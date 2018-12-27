#ifndef AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx
#define AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx

#include <THistogramArrayBase.h>

namespace bwf {

namespace hist {

class AbstractWaveform: public THistogramArrayBase {
public:

	AbstractWaveform();
	~AbstractWaveform() override;

	void UpdateHistograms(TDataContainer &dataContainer) override;

private:

	void createHistograms();

};

}

}

#endif   //	AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx
