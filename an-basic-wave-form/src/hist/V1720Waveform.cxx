#include <caen/v1720.hxx>
#include <midas/odb.hxx>
#include <fe-v1720.hxx>
#include "hist/V1720Waveform.hxx"

namespace bwf {

namespace hist {

V1720Waveform::V1720Waveform(VirtualOdb* const anOdb) :
		AbstractWaveform(anOdb, fe::v1720::equipName, fe::v1720::displayName,
				caen::v1720::nsPerSample<ns_per_sample_type>()) {

}

int V1720Waveform::loadWaveformLength(INT const feIndex) {
	auto const hKeyName = odb::equipSettingsKeyName(getBaseEquipName(), feIndex,
			fe::v1720::settings::waveformLength);
	auto const res = getOdb()->odbReadUint32(hKeyName.c_str(), 0, 0);
	return res > 0 ? res : -1;
}

}

}
