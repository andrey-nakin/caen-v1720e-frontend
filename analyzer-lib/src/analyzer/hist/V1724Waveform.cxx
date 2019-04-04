#include <caen/v1724.hxx>
#include <fe-v1724.hxx>
#include <analyzer/hist/V1724Waveform.hxx>

namespace analyzer {

namespace hist {

V1724Waveform::V1724Waveform(VirtualOdb* const anOdb,
		std::string const& anOdbRootKey) :
		DigitizerWaveform(anOdb, anOdbRootKey, fe::caen::v1724::equipName,
				fe::caen::v1724::displayName,
				caen::v1724::nsPerSample<ns_per_sample_type>()) {

}

void V1724Waveform::UpdateHistograms(TDataContainer &dataContainer) {

	using util::caen::V1724InfoRawData;

	auto const v1724Info = dataContainer.GetEventData < V1724InfoRawData
			> (V1724InfoRawData::bankName());
	if (v1724Info) {
		DigitizerWaveform::UpdateHistograms(dataContainer, *v1724Info);
	}

}

V1724Waveform::channel_no_type V1724Waveform::numOfChannels() const {

	return caen::v1724::NUM_OF_CHANNELS;

}

unsigned V1724Waveform::numOfSampleValues() const {

	return caen::v1724::NUM_OF_SAMPLE_VALUES;

}

uint16_t V1724Waveform::maxSampleValue() const {

	return caen::v1724::MAX_SAMPLE_VALUE;

}

}

}
