#include "fe/Frontend.hxx"
#include <midas/odb.hxx>
#include <midas/exception.hxx>
#include "util/FrontEndUtils.hxx"

#ifndef NEED_NO_EXTERN_C
extern "C" {
#endif

// declared in mfe.c
extern char exp_name[];
extern char **_argv;
extern INT frontend_index;

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

	util::FrontEndUtils::command([this]() {
		if (startsForTheFirstTime()) {
			doFirstRun();
		}

		doInit();
	});

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

INT Frontend::readEvent(char* const pevent, int const off) {

	return util::FrontEndUtils::commandR([this, pevent, off] {
		return doReadEvent(pevent, off);
	});

}

INT Frontend::interruptConfigure(INT /* cmd */, INT /* source */,
		PTYPE /* adr */) {

	return SUCCESS;

}

INT Frontend::onStop(INT const run_number, char* const error) {

	cm_msg(MDEBUG, frontend_name, "stop transition, run_number=%d", run_number);

	return util::FrontEndUtils::command([this, run_number, error] {
		return doOnStop(run_number, error);
	});

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

int Frontend::doReadEvent(char* /* pevent */, int /* off */) {

	return 0;

}

void Frontend::doOnStop(INT /* run_number */, char* /* error */) {

// null implementation

}

bool Frontend::startsForTheFirstTime() {

	auto const startCmd = odb::getValueString(hDB, 0,
			std::string("/Programs/") + frontend_name + "/Start command", "",
			false);
	return startCmd.empty();

}

static std::string baseFileName(std::string const& fn) {

	auto const pos = fn.find_last_of("/");
	if (pos == std::string::npos) {
		return fn;
	} else {
		return std::string(fn.begin() + pos + 1, fn.end());
	}

}

template<typename ValueT>
static std::string lpad(ValueT const v, std::size_t length) {

	std::string s = std::to_string(v);
	while (s.size() < length) {
		s = "0" + s;
	}
	return s;

}

void Frontend::doFirstRun() {

	std::string const feName = frontend_name
			+ (frontend_index < 0 ? "" : lpad(frontend_index, 2));

	odb::setValue(hDB, 0, std::string("/Programs/") + feName + "/Required",
			true);

	odb::setValue(hDB, 0, std::string("/Programs/") + feName + "/Start command",
			baseFileName(_argv[0]) + " -D -e " + exp_name
					+ (frontend_index < 0 ?
							"" : " -i " + std::to_string(frontend_index)));

}

void Frontend::doNoData() {

	ss_sleep(getSleepPeriod());

}

unsigned Frontend::getSleepPeriod() {

	return equipment[0].info.period;

}

}
