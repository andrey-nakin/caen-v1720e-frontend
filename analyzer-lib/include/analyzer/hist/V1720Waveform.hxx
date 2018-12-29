#ifndef ANALYZER_HIST_V1720Waveform_hxx
#define ANALYZER_HIST_V1720Waveform_hxx

#include <cstdint>
#include <VirtualOdb.h>
#include "AbstractWaveform.hxx"

namespace analyzer {

namespace hist {

class V1720Waveform: public AbstractWaveform {
public:

	V1720Waveform(VirtualOdb* anOdb);

	void UpdateHistograms(TDataContainer &dataContainer) override;

private:

//	unsigned loadWaveformLength(INT feIndex) override;
//	std::vector<bool> loadEnabledChannels(INT feIndex) override;

	template<typename IntT>
	static INT frontendIndex(IntT const v) {

		return v == static_cast<decltype(v)>(-1) ? -1 : static_cast<INT>(v);

	}

};

}

}

#endif   //	ANALYZER_HIST_V1720Waveform_hxx
