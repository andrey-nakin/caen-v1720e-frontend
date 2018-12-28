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

V1720Waveform::~V1720Waveform() {
}

unsigned V1720Waveform::loadWaveformLength(INT const feIndex) {

	auto const hKeyName = odb::equipSettingsKeyName(getBaseEquipName(), feIndex,
			fe::v1720::settings::waveformLength);

	if (getOdb()->odbReadArraySize(hKeyName.c_str()) <= 0) {
		return 0;
	}

	return getOdb()->odbReadUint32(hKeyName.c_str(), 0, 0);

}

std::vector<bool> V1720Waveform::loadEnabledChannels(INT feIndex) {

	std::vector<bool> result;
	auto const hKeyName = odb::equipSettingsKeyName(getBaseEquipName(), feIndex,
			fe::v1720::settings::enabledChannels);

	auto const size = getOdb()->odbReadArraySize(hKeyName.c_str());
	if (size > 0) {
		result.resize(size);

		for (int i = 0; i < size; i++) {
			result[i] = getOdb()->odbReadBool(hKeyName.c_str(), i, false);
		}
	}

	return result;

}

}

}
