#include "fe-v1724.hxx"

#ifndef NEED_NO_EXTERN_C
extern "C" {
#endif

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
const char *frontend_name = "fe-" EQUIP_NAME;

/* The frontend file name, don't change it */
const char *frontend_file_name = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = FALSE;

/* a frontend status page is displayed with this frequency in ms */
INT display_period = 1000;

/* maximum event size produced by this frontend */
INT max_event_size = fe::caen::v1724::MAX_EVENT_SIZE;
INT max_event_size_frag = fe::caen::v1724::MAX_EVENT_SIZE;

/* buffer size to hold events */
INT event_buffer_size = 2 * fe::caen::v1724::MAX_EVENT_SIZE;

/*-- Equipment list ------------------------------------------------*/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

EQUIPMENT equipment[] = { { EQUIP_NAME "%02d", { fe::caen::EVID, (1
		<< fe::caen::EVID), /* event ID, trigger mask */
"SYSTEM", /* event buffer */
EQ_MULTITHREAD, /* equipment type */
0, /* event source */
"MIDAS", /* format */
TRUE, /* enabled */
RO_RUNNING, /* Read when running */
10, /* poll every so milliseconds */
0, /* stop run after this event limit */
0, /* number of sub events */
0, /* no history */
"", "", "" }, readEvent, /* readout routine */
}, { "" } };

#pragma GCC diagnostic pop

INT onStop(INT run_number, char* error);

#ifndef NEED_NO_EXTERN_C
}
#endif

static ::fe::caen::v1724::V1724DigitizerFrontend frontend;

INT frontend_init() {

	// TODO cm_register_transition(TR_STOP, onStop, 500);
	return frontend.frontendInit();

}

INT frontend_exit() {

	return frontend.frontendExit();

}

INT begin_of_run(INT const run_number, char* const error) {

	return frontend.beginOfRun(run_number, error);

}

INT end_of_run(INT const run_number, char* const error) {

	return frontend.endOfRun(run_number, error);

}

INT pause_run(INT const run_number, char* const error) {

	return frontend.pauseRun(run_number, error);

}

INT resume_run(INT const run_number, char* const error) {

	return frontend.resumeRun(run_number, error);

}

INT frontend_loop() {

	return frontend.frontendLoop();

}

INT interrupt_configure(INT const cmd, INT const source, PTYPE const adr) {

	return frontend.interruptConfigure(cmd, source, adr);

}

INT poll_event(INT const source, INT const count, BOOL const test) {

	return frontend.pollEvent(source, count, test);

}

int readEvent(char * const pevent, int const off) {

	return frontend.readEvent(pevent, off);

}

INT onStop(INT const run_number, char* const error) {

	return frontend.onStop(run_number, error);

}
