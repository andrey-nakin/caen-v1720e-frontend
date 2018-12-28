#ifndef AN_BASIC_WAVE_FORM_HIST_V1720Waveform_hxx
#define AN_BASIC_WAVE_FORM_HIST_V1720Waveform_hxx

#include <cstdint>
#include <VirtualOdb.h>
#include "AbstractWaveform.hxx"

namespace bwf {

namespace hist {

class V1720Waveform: public AbstractWaveform<uint16_t> {
public:

	V1720Waveform(VirtualOdb* anOdb);

};

}

}

#endif   //	AN_BASIC_WAVE_FORM_HIST_V1720Waveform_hxx
