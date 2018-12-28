#ifndef AN_BASIC_WAVE_FORM_HIST_V1720Waveform_hxx
#define AN_BASIC_WAVE_FORM_HIST_V1720Waveform_hxx

#include <cstdint>
#include <VirtualOdb.h>
#include "AbstractWaveform.hxx"

namespace bwf {

namespace hist {

class V1720Waveform: public AbstractWaveform {
public:

	V1720Waveform(VirtualOdb* anOdb);

private:

	unsigned loadWaveformLength(INT feIndex) override;
	std::vector<bool> loadEnabledChannels(INT feIndex) override;

};

}

}

#endif   //	AN_BASIC_WAVE_FORM_HIST_V1720Waveform_hxx
