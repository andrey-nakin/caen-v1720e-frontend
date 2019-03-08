#ifndef ANALYZER_HIST_V1720Waveform_hxx
#define ANALYZER_HIST_V1720Waveform_hxx

#include <util/caen/V1720InfoRawData.hxx>
#include "DigitizerWaveform.hxx"

namespace analyzer {

namespace hist {

class V1720Waveform: public DigitizerWaveform {
public:

	V1720Waveform(VirtualOdb* anOdb);

	void UpdateHistograms(TDataContainer &dataContainer) override;

private:

	unsigned samplesPerTimeTick() const override;
	unsigned numOfChannels() const override;
	unsigned numOfSampleValues() const override;
	uint16_t maxSampleValue() const override;

	void AnalyzeWaveform(util::caen::DigitizerInfoRawData const* info,
			uint8_t channelNo, std::size_t numOfSamples,
			util::TWaveFormRawData::difference_type edgePosition,
			util::TWaveFormRawData::const_iterator_type wfBegin,
			util::TWaveFormRawData::const_iterator_type wfEnd);

	util::TWaveFormRawData::difference_type FindEdgeDistance(
			TDataContainer &dataContainer,
			util::caen::DigitizerInfoRawData const* info);

};

}

}

#endif   //	ANALYZER_HIST_V1720Waveform_hxx
