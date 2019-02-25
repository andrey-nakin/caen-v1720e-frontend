#pragma once

#include <midas/fe-api.hxx>

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
	virtual INT interruptConfigure(INT cmd, INT source, PTYPE adr);
	virtual INT pollEvent(INT source, INT count, BOOL test);
	virtual INT readEvent(char* pevent, int off);
	//

private:

	// Overridables
	virtual void doInit();
	virtual void doExit();
	virtual void doBeginOfRun(INT run_number, char* error);
	virtual void doEndOfRun(INT run_number, char* error);
	virtual void doPauseRun(INT run_number, char* error);
	virtual void doResumeRun(INT run_number, char* error);
	virtual void doLoop();
	virtual void doNoData();
	virtual int doPoll();
	virtual int doReadEvent(char* pevent, int off);

};

}
