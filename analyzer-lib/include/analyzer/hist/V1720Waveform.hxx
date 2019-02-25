#ifndef ANALYZER_HIST_V1720Waveform_hxx
#define ANALYZER_HIST_V1720Waveform_hxx

#include <cstdint>
#include <memory>
#include <fstream>
#include <VirtualOdb.h>
#include <util/caen/V1720InfoRawData.hxx>
#include <util/TWaveFormRawData.hxx>
#include "AbstractWaveform.hxx"

namespace analyzer {

namespace hist {

class V1720Waveform: public AbstractWaveform {
public:

	V1720Waveform(VirtualOdb* anOdb);

	void UpdateHistograms(TDataContainer &dataContainer) override;

private:

	uint16_t minFront;
	unsigned frontLength, peakLength;
	double threshold;
	bool rising;

//	unsigned loadWaveformLength(INT feIndex) override;
//	std::vector<bool> loadEnabledChannels(INT feIndex) override;

	template<typename IntT>
	static INT frontendIndex(IntT const v) {

		return v == static_cast<decltype(v)>(-1) ? -1 : static_cast<INT>(v);

	}

	void AnalyzeWaveform(util::caen::V1720InfoRawData const* v1720Info,
			uint8_t channelNo, std::size_t numOfSamples,
			util::TWaveFormRawData::difference_type edgePosition,
			util::TWaveFormRawData::const_iterator_type wfBegin,
			util::TWaveFormRawData::const_iterator_type wfEnd);

	util::TWaveFormRawData::difference_type FindEdgeDistance(TDataContainer&,
			util::caen::V1720InfoRawData const*);

	std::map<channel_type, std::vector<int32_t>> buffers;
	std::map<unsigned, std::unique_ptr<std::ofstream>> files;

};

}

}

#endif   //	ANALYZER_HIST_V1720Waveform_hxx
