#ifndef ANALYZER_HIST_AbstractWaveform_hxx
#define ANALYZER_HIST_AbstractWaveform_hxx

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

namespace analyzer {

namespace hist {

namespace settings {

constexpr char timeHistMaxBins[] = "time_hist_max_bins";
constexpr char ampHistMaxBins[] = "amp_hist_max_bins";

}

class AbstractWaveform: public THistogramArrayBase {
protected:

	typedef unsigned ns_per_sample_type;
	typedef unsigned channel_type;
	typedef TH1I HistType;

public:

	AbstractWaveform(VirtualOdb* anOdb, std::string const& anOdbRootKey,
			std::string const& aBaseEquipName, std::string const & aDisplayName,
			ns_per_sample_type const aNsPerSample);

	VirtualOdb* getOdb() const {

		return odb;

	}

	std::string const& getOdbRootKey() const {

		return odbRootKey;

	}

	std::string const& getBaseEquipName() const {

		return baseEquipName;

	}

//	void CreateHistograms();

	virtual void BeginRun(int transition, int run, int time);

	void ResetAllHistograms();

protected:

	HistType& GetWaveformHist(INT feIndex, unsigned channelNo,
			unsigned waveformLength);
	HistType& GetPositionHist(INT feIndex, unsigned channelNo,
			unsigned waveformLength, unsigned preTriggerLength);
	HistType& RebinPositionHist(INT feIndex, unsigned channelNo,
			unsigned waveformLength, unsigned preTriggerLength);
	HistType& GetAmplitudeHist(INT feIndex, unsigned channelNo,
			unsigned maxValue);

	void FillPositionHist(HistType& hist, int position,
			unsigned preTriggerLength);

	template<class InputIt>
	void SetData(HistType& h, InputIt const begin, InputIt const end) {

		Int_t bin = 1;
		std::for_each(begin, end, [&h, &bin](decltype(*begin) s) {
			h.SetBinContent(bin++, s);
		});

	}

	virtual unsigned GetPositionMaxBins() const;
	virtual unsigned GetAmplitudeMaxBins() const;

private:

	VirtualOdb* const odb;
	std::string const odbRootKey;
	std::string const baseEquipName;
	std::string const displayName;
	ns_per_sample_type const nsPerSample;
	std::map<INT, std::map<unsigned, HistType*>> wfHistograms, posHistograms,
			ampHistograms;
	std::map<INT, std::map<unsigned, bool>> histInitialized;

//	void createHistograms(INT feIndex);
	HistType* CreateWaveformHistogram(INT feIndex, unsigned channelNo,
			unsigned waveformLength);
	HistType* CreatePositionHistogram(INT feIndex, unsigned channelNo,
			unsigned waveformLength, unsigned preTriggerLength);
	HistType* CreateAmplitudeHistogram(INT feIndex, unsigned channelNo,
			unsigned maxValue);

	HistType& FindCreateWaveformHist(INT feIndex, unsigned channelNo,
			unsigned waveformLength);
	HistType& FindCreatePositionHist(INT feIndex, unsigned channelNo,
			unsigned waveformLength, unsigned preTriggerLength);
	HistType& FindCreateAmplitudeHist(INT feIndex, unsigned channelNo,
			unsigned maxValue);
	virtual void ResetHistogram(HistType& h, unsigned waveformLength);

//	virtual unsigned loadWaveformLength(INT feIndex) = 0;
//	virtual std::vector<bool> loadEnabledChannels(INT feIndex) = 0;

	virtual std::string ConstructName(INT feIndex, unsigned channelNo,
			const char* name);
	virtual std::string ConstructTitle(INT feIndex, unsigned channelNo,
			const char* title);

};

}

}

#endif   //	ANALYZER_HIST_AbstractWaveform_hxx
