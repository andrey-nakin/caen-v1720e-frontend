#ifndef AN_BASIC_WAVE_FORM_TAnaManager_hxx
#define AN_BASIC_WAVE_FORM_TAnaManager_hxx

#include <vector>
#include <TDataContainer.hxx>
#include <analyzer/hist/V1720Waveform.hxx>
#include <analyzer/hist/V1724Waveform.hxx>

namespace bwf {

namespace settings {

constexpr char resetHistograms[] = "reset_histograms";

}

class TAnaManager {

public:
	TAnaManager(VirtualOdb* anOdb, std::string const& anOdbRootKey);

	int ProcessMidasEvent(TDataContainer & dataContainer);

	void BeginRun(int transition, int run, int time);

	void EndRun(int transition, int run, int time);

private:

	VirtualOdb* odb;
	std::string odbRootKey;
	std::vector<std::unique_ptr<analyzer::hist::AbstractWaveform>> waveforms;

	void setResetHistogramsFlag(bool value = false);

	template<typename Executor>
	void ForEachWaveform(Executor executor) {

		for (auto i = waveforms.begin(); i != waveforms.end(); i++) {
			executor(i->get());
		}

	}

};

}
#endif   //      AN_BASIC_WAVE_FORM_TAnaManager_hxx

//~ Formatted by Jindent --- http://www.jindent.com
