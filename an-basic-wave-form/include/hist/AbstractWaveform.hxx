#ifndef AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx
#define AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx

#include <string>
#include <sstream>
#include <midas.h>
#include <VirtualOdb.h>
#include <util/types.hxx>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"

#include <THistogramArrayBase.h>

#pragma GCC diagnostic pop

namespace bwf {

namespace hist {

class AbstractWaveform: public THistogramArrayBase {
protected:

	typedef unsigned ns_per_sample_type;

public:

	AbstractWaveform(VirtualOdb* anOdb, std::string const& aBaseEquipName,
			std::string const & aDisplayName,
			ns_per_sample_type const aNsPerSample);

	void UpdateHistograms(TDataContainer &dataContainer) override;

	VirtualOdb* getOdb() const {

		return odb;

	}

	std::string const& getBaseEquipName() const {

		return baseEquipName;

	}

private:

	VirtualOdb* const odb;
	std::string const baseEquipName;
	std::string const displayName;
	ns_per_sample_type const nsPerSample;

	void createHistograms();

	void createHistograms(INT feIndex);

	virtual int loadWaveformLength(INT feIndex) = 0;

};

}

}

#endif   //	AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx
