#pragma once

#include <mutex>
#include "Frontend.hxx"

namespace fe {

class SynchronizedFrontend: public Frontend {
private:

	void doInit() override;
	void doExit() override;
	void doBeginOfRun(INT run_number, char* error) override;
	void doEndOfRun(INT run_number, char* error) override;
	void doPauseRun(INT run_number, char* error) override;
	void doResumeRun(INT run_number, char* error) override;
	void doLoop() override;
	int doPoll() override;
	int doReadEvent(char* pevent, int off) override;
	void doOnStop(INT run_number, char* error) override;

	// Overridables
	virtual void doInitSynchronized();
	virtual void doExitSynchronized();
	virtual void doBeginOfRunSynchronized(INT run_number, char* error);
	virtual void doEndOfRunSynchronized(INT run_number, char* error);
	virtual void doPauseRunSynchronized(INT run_number, char* error);
	virtual void doResumeRunSynchronized(INT run_number, char* error);
	virtual void doLoopSynchronized();
	virtual int doPollSynchronized();
	virtual int doReadEventSynchronized(char* pevent, int off);
	virtual void doOnStopSynchronized(INT run_number, char* error);

private:

	std::recursive_mutex mutex;

};

}
