#include "hist/AbstractWaveform.hxx"

namespace bwf {

namespace hist {

AbstractWaveform::AbstractWaveform() {

	createHistograms();

}

AbstractWaveform::~AbstractWaveform() {

}

void AbstractWaveform::UpdateHistograms(TDataContainer &dataContainer) {

}

void AbstractWaveform::createHistograms() {

	auto const h = new TH1D("V1720", "V1720 Waveform", 1024, 0, 1024 * 4);
	h->SetXTitle("ns");
	h->SetYTitle("ADC Value");
	push_back(h);

}

}

}
