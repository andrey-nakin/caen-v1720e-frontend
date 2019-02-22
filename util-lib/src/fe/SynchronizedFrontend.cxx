#include "fe/SynchronizedFrontend.hxx"

namespace fe {

void SynchronizedFrontend::doInit() {

	std::lock_guard<decltype(mutex)> lock(mutex);
	doInitSynchronized();

}

void SynchronizedFrontend::doExit() {

	std::lock_guard<decltype(mutex)> lock(mutex);
	doExitSynchronized();

}

void SynchronizedFrontend::doBeginOfRun(INT const run_number,
		char* const error) {

	std::lock_guard<decltype(mutex)> lock(mutex);
	doBeginOfRunSynchronized(run_number, error);

}

void SynchronizedFrontend::doEndOfRun(INT const run_number, char* const error) {

	std::lock_guard<decltype(mutex)> lock(mutex);
	doEndOfRunSynchronized(run_number, error);

}

void SynchronizedFrontend::doPauseRun(INT const run_number, char* const error) {

	std::lock_guard<decltype(mutex)> lock(mutex);
	doPauseRunSynchronized(run_number, error);

}

void SynchronizedFrontend::doResumeRun(INT const run_number,
		char* const error) {

	std::lock_guard<decltype(mutex)> lock(mutex);
	doResumeRunSynchronized(run_number, error);

}

void SynchronizedFrontend::doLoop() {

	std::lock_guard<decltype(mutex)> lock(mutex);
	doLoopSynchronized();

}

int SynchronizedFrontend::doPoll() {

	std::lock_guard<decltype(mutex)> lock(mutex);
	return doPollSynchronized();

}

void SynchronizedFrontend::doInitSynchronized() {

	// null implementation

}

void SynchronizedFrontend::doExitSynchronized() {

	// null implementation

}

void SynchronizedFrontend::doBeginOfRunSynchronized(INT /* run_number */,
		char* /* error */) {

	// null implementation

}

void SynchronizedFrontend::doEndOfRunSynchronized(INT /* run_number */,
		char* /* error */) {

	// null implementation

}

void SynchronizedFrontend::doPauseRunSynchronized(INT /* run_number */,
		char* /* error */) {

	// null implementation

}

void SynchronizedFrontend::doResumeRunSynchronized(INT /* run_number */,
		char* /* error */) {

	// null implementation

}
void SynchronizedFrontend::doLoopSynchronized() {

	// null implementation
}

int SynchronizedFrontend::doPollSynchronized() {

	return 0;

}

}
