#ifndef AN_BASIC_WAVE_FORM_TAnaManager_hxx
#define AN_BASIC_WAVE_FORM_TAnaManager_hxx

#include <TDataContainer.hxx>

namespace bwf {

class TAnaManager {
public:
	TAnaManager();

	int ProcessMidasEvent(TDataContainer& dataContainer);

	void BeginRun(int transition, int run, int time);
	void EndRun(int transition, int run, int time);

private:

};

}

#endif	//	AN_BASIC_WAVE_FORM_TAnaManager_hxx

