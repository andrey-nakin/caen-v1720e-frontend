#pragma once

#include <util/caen/V1724InfoRawData.hxx>
#include "DigitizerWaveform.hxx"

namespace analyzer {

namespace hist {

class V1724Waveform: public DigitizerWaveform {
public:

	V1724Waveform(VirtualOdb* anOdb, std::string const& anOdbRootKey);

	void UpdateHistograms(TDataContainer &dataContainer) override;

private:

	unsigned samplesPerTimeTick() const override;
	channel_no_type numOfChannels() const override;
	unsigned numOfSampleValues() const override;
	uint16_t maxSampleValue() const override;

};

}

}
