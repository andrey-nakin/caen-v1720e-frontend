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
		odb(anOdb), odbRootKey(anOdbRootKey), v1720Waveform(anOdb,
				anOdbRootKey), v1724Waveform(anOdb, anOdbRootKey) {

}

int TAnaManager::ProcessMidasEvent(TDataContainer& dataContainer) {

	auto midasOdb = dynamic_cast<TMidasOnline*>(odb);
	if (midasOdb) {
		auto const key = analyzer::util::AnalyzerUtils::OdbKey(odbRootKey,
				settings::resetHistograms);
		auto const doReset = midasOdb->odbReadBool(key.c_str(), 0, false);
		if (doReset) {
			v1720Waveform.ResetAllHistograms();
			v1724Waveform.ResetAllHistograms();
		}

		setResetHistogramsFlag();
	}

	v1720Waveform.UpdateHistograms(dataContainer);
	v1724Waveform.UpdateHistograms(dataContainer);

	return 1;
}

void TAnaManager::BeginRun(int const transition, int const run,
		int const time) {

	setResetHistogramsFlag();

	v1720Waveform.BeginRun(transition, run, time);
	v1724Waveform.BeginRun(transition, run, time);

}

void TAnaManager::EndRun(int const transition, int const run, int const time) {

	v1720Waveform.EndRun(transition, run, time);
	v1724Waveform.EndRun(transition, run, time);

}

void TAnaManager::setResetHistogramsFlag(bool value) {

	auto midasOdb = dynamic_cast<TMidasOnline*>(odb);
	if (midasOdb) {
		auto const key = analyzer::util::AnalyzerUtils::OdbKey(odbRootKey,
				settings::resetHistograms);
		::odb::setValue(midasOdb->fDB, 0, key, value);
	}

}

}
