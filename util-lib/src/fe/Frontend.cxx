#include "fe/Frontend.hxx"

#include <midas/exception.hxx>
#include "util/FrontEndUtils.hxx"

#ifndef NEED_NO_EXTERN_C
extern "C" {
#endif

extern const char *frontend_name;
extern EQUIPMENT equipment[];
extern HNDLE hDB;

#ifndef NEED_NO_EXTERN_C
}
#endif

namespace fe {

Frontend::Frontend() {

}

Frontend::~Frontend() {

}

INT Frontend::frontendInit() {

	cm_msg(MDEBUG, frontend_name, "frontend_init");

	midas::Exception::setProgramName(frontend_name);

	util::FrontEndUtils::command([this]() {doInit();});

	return SUCCESS;

}

INT Frontend::frontendExit() {

	cm_msg(MDEBUG, frontend_name, "frontend_exit");

	return util::FrontEndUtils::command([this]() {doExit();});

}

INT Frontend::beginOfRun(INT const run_number, char* const error) {

	cm_msg(MDEBUG, frontend_name, "begin_of_run run_number=%d", run_number);

	return util::FrontEndUtils::command([this, run_number, error]() {
		doBeginOfRun(run_number, error);
	});

}

INT Frontend::endOfRun(INT const run_number, char* const error) {

	cm_msg(MDEBUG, frontend_name, "end_of_run run_number=%d", run_number);

	return util::FrontEndUtils::command([this, run_number, error] {
		doEndOfRun(run_number, error);
	});

}

INT Frontend::pauseRun(INT const run_number, char* const error) {

	cm_msg(MDEBUG, frontend_name, "pause_run run_number=%d", run_number);

	return util::FrontEndUtils::command([this, run_number, error]() {
		doPauseRun(run_number, error);
	});

}

INT Frontend::resumeRun(INT const run_number, char* const error) {

	cm_msg(MDEBUG, frontend_name, "resume_run run_number=%d", run_number);

	return util::FrontEndUtils::command([this, run_number, error]() {
		doResumeRun(run_number, error);
	});

}

INT Frontend::frontendLoop() {

	return util::FrontEndUtils::command([this]() {
		doLoop();
	});

}

INT Frontend::pollEvent(INT const /* source */, INT const /* count */,
		BOOL const test) {

	INT const result = util::FrontEndUtils::commandR([this] {
		return doPoll();
	});

	if (!result) {
		doNoData();
	}

	return result && !test;

}

INT Frontend::interruptConfigure(INT /* cmd */, INT /* source */,
		PTYPE /* adr */) {

	return SUCCESS;

}

void Frontend::doInit() {

	// null implementation

}

void Frontend::doExit() {

	// null implementation

}

void Frontend::doBeginOfRun(INT /* run_number */, char* /* error */) {

	// null implementation

}

void Frontend::doEndOfRun(INT /* run_number */, char* /* error */) {

	// null implementation

}

void Frontend::doPauseRun(INT /* run_number */, char* /* error */) {

	// null implementation

}

void Frontend::doResumeRun(INT /* run_number */, char* /* error */) {

	// null implementation

}

void Frontend::doLoop() {

	// null implementation

}

int Frontend::doPoll() {

	return 0;

}

void Frontend::doNoData() {

	ss_sleep(equipment[0].info.period);

}

}
