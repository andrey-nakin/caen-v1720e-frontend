#include <iostream>
#include <midas.h>
//#include <caen/handle.hxx>
#include <stdio.h>

/* make frontend functions callable from the C framework */
#ifdef __cplusplus
extern "C" {
#endif

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
char frontend_name[] = "fe_v1720";
/* The frontend file name, don't change it */
char frontend_file_name[] = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = TRUE;

/* a frontend status page is displayed with this frequency in ms */
INT display_period = 1000;

/* maximum event size produced by this frontend */
INT max_event_size = 100 * 1024;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 200 * 1024;

extern INT run_state;
extern HNDLE hDB;

/*-- Function declarations -----------------------------------------*/
INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();

INT read_event(char *pevent, INT off);
INT poll_event(INT source, INT count, BOOL test);
INT interrupt_configure(INT cmd, INT source, PTYPE adr);

/*-- Bank definitions ----------------------------------------------*/

/*-- Equipment list ------------------------------------------------*/

EQUIPMENT equipment[] = { { "" } };

//EQUIPMENT equipment[] = {
//
//{ "Trigger", /* equipment name */
//{ 1, TRIGGER_ALL, /* event ID, trigger mask */
//"SYSTEM", /* event buffer */
//EQ_POLLED, /* equipment type */
//LAM_SOURCE(0, 0xFFFFFF), /* event source */
//"MIDAS", /* format */
//TRUE, /* enabled */
//RO_RUNNING, /* read only when running */
//
//500, /* poll for 500ms */
//0, /* stop run after this event limit */
//0, /* number of sub events */
//0, /* don't log history */
//"", "", "", }, read_event, /* readout routine */
//NULL, NULL,
//NULL, /* bank list */
//},
//
//{ "" } };

#ifdef __cplusplus
}
#endif

/*-- Frontend Init -------------------------------------------------*/

INT frontend_init() {
	printf("frontend_init()\n");
	std::cout << "frontend_init() hDB=" << hDB << std::endl;
	INT status = SUCCESS;

//	set_equipment_status(equipment[0].name, "Initializing...", "yellow");

	return status;
}

/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit() {
	std::cout << "frontend_exit()" << std::endl;

	return SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/

INT begin_of_run(INT run_number, char *error) {
	std::cout << "begin_of_run(" << run_number << ", " << error << ")"
			<< std::endl;
	return SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/
INT end_of_run(INT run_number, char *error) {
	std::cout << "end_of_run(" << run_number << ", " << error << ")"
			<< std::endl;
	return SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/
INT pause_run(INT run_number, char *error) {
	std::cout << "pause_run(" << run_number << ", " << error << ")"
			<< std::endl;
	return SUCCESS;
}

/*-- Resume Run ----------------------------------------------------*/
INT resume_run(INT run_number, char *error) {
	std::cout << "resume_run(" << run_number << ", " << error << ")"
			<< std::endl;
	return SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/
INT frontend_loop() {
	std::cout << "frontend_loop()" << std::endl;
	return SUCCESS;
}

/*-- Trigger event routines ----------------------------------------*/
INT poll_event(INT source, INT count, BOOL test) {
	std::cout << "poll_event()" << std::endl;
	return FALSE;
}

/*-- Interrupt configuration ---------------------------------------*/
INT interrupt_configure(INT cmd, INT source, PTYPE adr) {
	std::cout << "interrupt_configure()" << std::endl;
	switch (cmd) {
	case CMD_INTERRUPT_ENABLE:
		break;
	case CMD_INTERRUPT_DISABLE:
		break;
	case CMD_INTERRUPT_ATTACH:
		break;
	case CMD_INTERRUPT_DETACH:
		break;
	}
	return SUCCESS;
}

/*-- Read Event -------------------------------------------------*/
INT read_event(char *pevent, INT off) {
	std::cout << "read_event()" << std::endl;
	return 0;
}
