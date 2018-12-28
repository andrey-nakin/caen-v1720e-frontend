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

	virtual int loadWaveformLength(INT feIndex) override;

};

}

}

#endif   //	AN_BASIC_WAVE_FORM_HIST_V1720Waveform_hxx
