#include <caen/v1720.hxx>
#include <fe-v1720.hxx>
#include <analyzer/hist/V1720Waveform.hxx>

namespace analyzer {

namespace hist {

V1720Waveform::V1720Waveform(VirtualOdb* const anOdb,
		std::string const& anOdbRootKey) :
		DigitizerWaveform(anOdb, anOdbRootKey, fe::caen::v1720::equipName,
				fe::caen::v1720::displayName,
				caen::v1720::nsPerSample<ns_per_sample_type>()) {

}

void V1720Waveform::UpdateHistograms(TDataContainer &dataContainer) {

	using util::caen::V1720InfoRawData;

	auto const v1720Info = dataContainer.GetEventData < V1720InfoRawData
			> (V1720InfoRawData::bankName());
	if (v1720Info) {
		DigitizerWaveform::UpdateHistograms(dataContainer, *v1720Info);
	}

}

V1720Waveform::channel_no_type V1720Waveform::numOfChannels() const {

	return caen::v1720::NUM_OF_CHANNELS;

}

unsigned V1720Waveform::numOfSampleValues() const {

	return caen::v1720::NUM_OF_SAMPLE_VALUES;

}

uint16_t V1720Waveform::maxSampleValue() const {

	return caen::v1720::MAX_SAMPLE_VALUE;

}

}

}
