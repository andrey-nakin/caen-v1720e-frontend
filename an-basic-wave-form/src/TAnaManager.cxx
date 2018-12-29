#include <sstream>
#include <fstream>
#include <memory>
#include <TGraph.h>
#include <TCanvas.h>
#include "TAnaManager.hxx"

namespace bwf {

TAnaManager::TAnaManager(VirtualOdb* const odb) :
		v1720Waveform(odb) {

}

int TAnaManager::ProcessMidasEvent(TDataContainer& dataContainer) {

	v1720Waveform.UpdateHistograms(dataContainer);

	return 1;
}

void TAnaManager::BeginRun(int const transition, int const run,
		int const time) {

	v1720Waveform.BeginRun(transition, run, time);

}

void TAnaManager::EndRun(int /* transition */, int /* run */, int /* time */) {
}

}
