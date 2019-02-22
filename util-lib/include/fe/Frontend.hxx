#ifndef	UTIL_LIB_FE_Frontend_hxx
#define	UTIL_LIB_FE_Frontend_hxx

#include <midas.h>

namespace fe {

class Frontend {
public:

	Frontend();
	Frontend(const Frontend&) = delete;

	virtual ~Frontend();

	Frontend& operator=(const Frontend&) = delete;

	// MIDAS API
	virtual INT frontendInit();
	virtual INT frontendExit();
	virtual INT beginOfRun(INT run_number, char* error);
	virtual INT endOfRun(INT run_number, char* error);
	virtual INT pauseRun(INT run_number, char* error);
	virtual INT resumeRun(INT run_number, char* error);
	virtual INT frontendLoop();
	virtual INT pollEvent(INT source, INT count, BOOL test);
	virtual INT interruptConfigure(INT cmd, INT source, PTYPE adr);
	//

	// Overridables
	virtual void doInit();
	virtual void doExit();
	virtual void doBeginOfRun(INT run_number, char* error);
	virtual void doEndOfRun(INT run_number, char* error);
	virtual void doPauseRun(INT run_number, char* error);
	virtual void doResumeRun(INT run_number, char* error);
	virtual void doLoop();
	virtual int doPoll();
	virtual void doNoData();

}
;

}

#endif	//	UTIL_LIB_FE_Frontend_hxx

