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

void SynchronizedFrontend::doYield() {

	std::lock_guard<decltype(mutex)> lock(mutex);
	doYieldSynchronized();

}

int SynchronizedFrontend::doPoll() {

	std::lock_guard<decltype(mutex)> lock(mutex);
	return doPollSynchronized();

}

int SynchronizedFrontend::doReadEvent(char* const pevent, int const off) {

	std::lock_guard<decltype(mutex)> lock(mutex);
	return doReadEventSynchronized(pevent, off);

}

void SynchronizedFrontend::doOnStop(INT const run_number, char* const error) {

	std::lock_guard<decltype(mutex)> lock(mutex);
	doOnStopSynchronized(run_number, error);

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

void SynchronizedFrontend::doYieldSynchronized() {

	// null implementation

}

int SynchronizedFrontend::doPollSynchronized() {

	return 0;

}

int SynchronizedFrontend::doReadEventSynchronized(char* /* pevent */,
		int /* off */) {

	return 0;

}

void SynchronizedFrontend::doOnStopSynchronized(INT /* run_number */,
		char* /* error */) {

	// null implementation

}

}
