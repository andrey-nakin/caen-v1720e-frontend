#ifndef AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx
#define AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx

#include <string>
#include <vector>
#include <map>
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
	typedef TH1D HistType;

public:

	AbstractWaveform(AbstractWaveform const&) = delete;
	AbstractWaveform(VirtualOdb* anOdb, std::string const& aBaseEquipName,
			std::string const & aDisplayName,
			ns_per_sample_type const aNsPerSample);
	~AbstractWaveform() override;

	AbstractWaveform& operator=(AbstractWaveform const&) = delete;

	VirtualOdb* getOdb() const {

		return odb;

	}

	std::string const& getBaseEquipName() const {

		return baseEquipName;

	}

	void CreateHistograms();

protected:

	HistType* GetHist(INT feIndex, unsigned channelNo) const;

private:

	VirtualOdb* const odb;
	std::string const baseEquipName;
	std::string const displayName;
	ns_per_sample_type const nsPerSample;
	std::map<INT, std::map<unsigned, HistType*>> histograms;

	void createHistograms(INT feIndex);

	virtual unsigned loadWaveformLength(INT feIndex) = 0;
	virtual std::vector<bool> loadEnabledChannels(INT feIndex) = 0;

	virtual std::string ConstructName(INT feIndex, unsigned channelNo);
	virtual std::string ConstructTitle(INT feIndex, unsigned channelNo);

};

}

}

#endif   //	AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx
