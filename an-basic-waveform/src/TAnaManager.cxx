#include <sstream>
#include <fstream>
#include <memory>
#include <TGraph.h>
#include <TCanvas.h>
#include "TAnaManager.hxx"

namespace bwf {

TAnaManager::TAnaManager(VirtualOdb* const odb, std::string const& anOdbRootKey) :
		v1720Waveform(odb, anOdbRootKey), v1724Waveform(odb, anOdbRootKey) {

}

int TAnaManager::ProcessMidasEvent(TDataContainer& dataContainer) {

	v1720Waveform.UpdateHistograms(dataContainer);
	v1724Waveform.UpdateHistograms(dataContainer);

	return 1;
}

void TAnaManager::BeginRun(int const transition, int const run,
		int const time) {

	v1720Waveform.BeginRun(transition, run, time);
	v1724Waveform.BeginRun(transition, run, time);

}

void TAnaManager::EndRun(int const transition, int const run, int const time) {

	v1720Waveform.EndRun(transition, run, time);
	v1724Waveform.EndRun(transition, run, time);

}

}
