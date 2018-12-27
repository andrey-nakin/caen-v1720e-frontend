#include <caen/v1720.hxx>
#include <fe-v1720.hxx>
#include "hist/V1720Waveform.hxx"

namespace bwf {

namespace hist {

V1720Waveform::V1720Waveform() :
		AbstractWaveform(EQUIP_NAME, "V1720", caen::v1720::NS_IN_SAMPLE_INT) {

}

}

}
