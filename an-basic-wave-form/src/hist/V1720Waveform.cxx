#include <caen/v1720.hxx>
#include <fe-v1720.hxx>
#include "hist/V1720Waveform.hxx"

namespace bwf {

namespace hist {

V1720Waveform::V1720Waveform(VirtualOdb* const anOdb) :
		AbstractWaveform(anOdb, fe::v1720::equipName, fe::v1720::displayName,
				caen::v1720::nsPerSample<ns_per_sample_type>()) {

}

}

}
