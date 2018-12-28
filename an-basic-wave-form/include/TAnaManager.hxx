#ifndef AN_BASIC_WAVE_FORM_TAnaManager_hxx
#define AN_BASIC_WAVE_FORM_TAnaManager_hxx

#include <memory>
#include <TDataContainer.hxx>
#include "hist/V1720Waveform.hxx"

namespace bwf {

class TAnaManager {

public:
	TAnaManager(VirtualOdb* anOdb);

	int ProcessMidasEvent(TDataContainer & dataContainer);

	void BeginRun(int transition, int run, int time);

	void EndRun(int transition, int run, int time);

private:

	VirtualOdb* const odb;
	std::unique_ptr<hist::V1720Waveform> v1720Waveform;

};

}
#endif   //      AN_BASIC_WAVE_FORM_TAnaManager_hxx

//~ Formatted by Jindent --- http://www.jindent.com
