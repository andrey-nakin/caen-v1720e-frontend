#ifndef AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx
#define AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx

#include <string>
#include <vector>
#include <map>
#include <algorithm>
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

//	void CreateHistograms();

	virtual void BeginRun(int transition, int run, int time);

protected:

	HistType& GetHist(INT feIndex, unsigned channelNo, unsigned waveformLength);

	template<class InputIt>
	void SetData(HistType& h, InputIt const begin, InputIt const end) {

		h.Reset();

		Int_t bin = 1;
		std::for_each(begin, end, [&h, &bin](decltype(*begin) s) {
			h.SetBinContent(bin++, s);
		});

	}

private:

	VirtualOdb* const odb;
	std::string const baseEquipName;
	std::string const displayName;
	ns_per_sample_type const nsPerSample;
	std::map<INT, std::map<unsigned, HistType*>> histograms;
	std::map<INT, std::map<unsigned, bool>> histInitialized;

//	void createHistograms(INT feIndex);
	HistType* CreateHistogram(INT feIndex, unsigned channelNo,
			unsigned waveformLength);
	HistType& FindCreateHist(INT feIndex, unsigned channelNo,
			unsigned waveformLength);
	void ResetHistogram(HistType& h, unsigned waveformLength);

//	virtual unsigned loadWaveformLength(INT feIndex) = 0;
//	virtual std::vector<bool> loadEnabledChannels(INT feIndex) = 0;

	virtual std::string ConstructName(INT feIndex, unsigned channelNo);
	virtual std::string ConstructTitle(INT feIndex, unsigned channelNo);

};

}

}

#endif   //	AN_BASIC_WAVE_FORM_HIST_AbstractWaveform_hxx
