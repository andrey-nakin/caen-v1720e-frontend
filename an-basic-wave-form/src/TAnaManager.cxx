#include <sstream>
#include <fstream>
#include <memory>
#include <TGraph.h>
#include <TCanvas.h>
#include "TAnaManager.hxx"

namespace bwf {

TAnaManager::TAnaManager(VirtualOdb* const anOdb) :
		odb(anOdb) {

	v1720Waveform.reset(new hist::V1720Waveform(anOdb));

}

int TAnaManager::ProcessMidasEvent(TDataContainer& dataContainer) {

	v1720Waveform->UpdateHistograms(dataContainer);

	return 1;
}

void TAnaManager::BeginRun(int const transition, int const run,
		int const time) {

	v1720Waveform->BeginRun(transition, run, time);

}

void TAnaManager::EndRun(int /* transition */, int /* run */, int /* time */) {
}

}
