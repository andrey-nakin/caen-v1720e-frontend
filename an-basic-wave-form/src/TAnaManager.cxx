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
	v1720Waveform->CreateHistograms();

}

int TAnaManager::ProcessMidasEvent(TDataContainer& dataContainer) {

	v1720Waveform->UpdateHistograms(dataContainer);

	return 1;
}

void TAnaManager::BeginRun(int /* transition */, int /* run */,
		int /* time */) {

	v1720Waveform->CreateHistograms();

}

void TAnaManager::EndRun(int /* transition */, int /* run */, int /* time */) {
}

}
