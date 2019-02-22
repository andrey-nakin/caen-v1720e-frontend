#ifndef	UTIL_LIB_FE_SynchronizedFrontend_hxx
#define	UTIL_LIB_FE_SynchronizedFrontend_hxx

#include <mutex>
#include "Frontend.hxx"

namespace fe {

class SynchronizedFrontend: public Frontend {
public:

	void doInit() override;
	void doExit() override;
	void doBeginOfRun(INT run_number, char* error) override;
	void doEndOfRun(INT run_number, char* error) override;
	void doPauseRun(INT run_number, char* error) override;
	void doResumeRun(INT run_number, char* error) override;
	void doLoop() override;
	int doPoll() override;

	// Overridables
	virtual void doInitSynchronized();
	virtual void doExitSynchronized();
	virtual void doBeginOfRunSynchronized(INT run_number, char* error);
	virtual void doEndOfRunSynchronized(INT run_number, char* error);
	virtual void doPauseRunSynchronized(INT run_number, char* error);
	virtual void doResumeRunSynchronized(INT run_number, char* error);
	virtual void doLoopSynchronized();
	virtual int doPollSynchronized();

private:

	std::recursive_mutex mutex;

};

}

#endif	//	UTIL_LIB_FE_SynchronizedFrontend_hxx

