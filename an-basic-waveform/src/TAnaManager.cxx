	#include <sstream>
#include <fstream>
#include <memory>
#include <TGraph.h>
#include <TCanvas.h>
#include <TMidasOnline.h>
#include "TAnaManager.hxx"
#include <midas/odb.hxx>
#include <analyzer/util/AnalyzerUtils.hxx>

namespace bwf {

TAnaManager::TAnaManager(VirtualOdb* const anOdb,
		std::string const& anOdbRootKey) :
		odb(anOdb), odbRootKey(anOdbRootKey) {

	waveforms.push_back(
			std::unique_ptr < analyzer::hist::AbstractWaveform
					> (new analyzer::hist::V1720Waveform(anOdb, anOdbRootKey)));
	waveforms.push_back(
			std::unique_ptr < analyzer::hist::AbstractWaveform
					> (new analyzer::hist::V1724Waveform(anOdb, anOdbRootKey)));

}

int TAnaManager::ProcessMidasEvent(TDataContainer& dataContainer) {

	auto midasOdb = dynamic_cast<TMidasOnline*>(odb);
	if (midasOdb) {
		auto const key = analyzer::util::AnalyzerUtils::OdbKey(odbRootKey,
				settings::resetHistograms);
		auto const doReset = midasOdb->odbReadBool(key.c_str(), 0, false);
		if (doReset) {
			ForEachWaveform([](analyzer::hist::AbstractWaveform* wf) {
				wf->ResetAllHistograms();
			});
		}

		setResetHistogramsFlag();
	}

	ForEachWaveform([&dataContainer](analyzer::hist::AbstractWaveform* wf) {
		wf->UpdateHistograms(dataContainer);
	});

	return 1;
}

void TAnaManager::BeginRun(int const transition, int const run,
		int const time) {

	setResetHistogramsFlag();

	ForEachWaveform(
			[transition, run, time](analyzer::hist::AbstractWaveform* wf) {
				wf->BeginRun(transition, run, time);
			});

}

void TAnaManager::EndRun(int const transition, int const run, int const time) {

	ForEachWaveform(
			[transition, run, time](analyzer::hist::AbstractWaveform* wf) {
				wf->EndRun(transition, run, time);
			});

}

void TAnaManager::setResetHistogramsFlag(bool value) {

	std::string msg = std::string("setResetHistogramsFlag ") + typeid(*odb).name();
	cm_msg(MERROR, "TAnaManager", msg.c_str(), 0);

	auto midasOdb = dynamic_cast<TMidasOnline*>(odb);
	if (midasOdb) {
		auto const key = analyzer::util::AnalyzerUtils::OdbKey(odbRootKey,
				settings::resetHistograms);
		::odb::setValue(midasOdb->fDB, 0, key, value);
	}

}

}
