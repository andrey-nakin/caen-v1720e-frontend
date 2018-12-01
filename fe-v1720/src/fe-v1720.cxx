#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstddef>

#include <midas.h>

#include <midas/odb.hxx>
#include <frontend/types.hxx>
#include "defaults.hxx"

typedef struct {
	int Channels;
} CAEN_DGTZ_BoardInfo_t;

namespace globals {

CAEN_DGTZ_BoardInfo_t boardInfo;

}

#define EQUIP_NAME "v1720"

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
//INT display_period = 3000;
INT display_period = 0;

/* maximum event size produced by this frontend */
INT max_event_size = 4 * 1024 * 1024;
INT max_event_size_frag = 4 * 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 10 * 1024 * 1024;

/*-- Function declarations -----------------------------------------*/

INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();
int create_event_rb(int i);
int get_event_rb(int i);
extern int stop_all_threads;
INT poll_event(INT source, INT count, BOOL test);
INT interrupt_configure(INT cmd, INT source, PTYPE adr);

extern HNDLE hDB;

int read_test_event(char *pevent, int off);
int read_slow_event(char *pevent, int off);
int read_random_event(char *pevent, int off);

/*-- Equipment list ------------------------------------------------*/

#define EVID_TEST 1
#define EVID_SLOW 2
#define EVID_RANDOM 3

EQUIPMENT equipment[] = {

{ EQUIP_NAME, {
EVID_TEST, (1 << EVID_TEST), /* event ID, trigger mask */
"SYSTEM", /* event buffer */
EQ_USER, /* equipment type */
0, /* event source */
"MIDAS", /* format */
TRUE, /* enabled */
RO_RUNNING, /* Read when running */
10, /* poll every so milliseconds */
0, /* stop run after this event limit */
0, /* number of sub events */
0, /* no history */
"", "", "" }, read_test_event, /* readout routine */
}, { "" } };

#ifndef NEED_NO_EXTERN_C
}
#endif

/********************************************************************\
              Callback routines for system transitions

 These routines are called whenever a system transition like start/
 stop of a run occurs. The routines are called on the following
 occations:

 frontend_init:  When the frontend program is started. This routine
 should initialize the hardware.

 frontend_exit:  When the frontend program is shut down. Can be used
 to releas any locked resources like memory, commu-
 nications ports etc.

 begin_of_run:   When a new run is started. Clear scalers, open
 rungates, etc.

 end_of_run:     Called on a request to stop a run. Can send
 end-of-run event and close run gates.

 pause_run:      When a run is paused. Should disable trigger events.

 resume_run:     When a run is resumed. Should enable trigger events.

 \********************************************************************/

int event_size = 10 * 1024;

extern "C" {
void set_rate_period(int ms);
}

/*-- Frontend Init -------------------------------------------------*/

HNDLE hSet;
int test_rb_wait_sleep = 1;

// RPC handler

INT rpc_callback(INT index, void *prpc_param[]) {
	const char* cmd = CSTRING(0);
	const char* args = CSTRING(1);
	char* return_buf = CSTRING(2);
	int return_max_length = CINT(3);

	cm_msg(MINFO, "rpc_callback",
			"--------> rpc_callback: index %d, max_length %d, cmd [%s], args [%s]",
			index, return_max_length, cmd, args);

	//int example_int = strtol(args, NULL, 0);
	//int size = sizeof(int);
	//int status = db_set_value(hDB, 0, "/Equipment/" EQ_NAME "/Settings/example_int", &example_int, size, 1, TID_INT);

	char tmp[256];
	time_t now = time(NULL);
	sprintf(tmp, "{ \"current_time\" : [ %d, \"%s\"] }", (int) now,
			ctime(&now));

	strlcpy(return_buf, tmp, return_max_length);

	return RPC_SUCCESS;
}

void configure() {
	int size, status;

	size = sizeof(event_size);
	status = db_get_value(hDB, hSet, "event_size", &event_size, &size,
			TID_DWORD, TRUE);
	assert(status == DB_SUCCESS);

	printf("Event size set to %d bytes\n", event_size);

	size = sizeof(test_rb_wait_sleep);
	status = db_get_value(hDB, hSet, "rb_wait_sleep", &test_rb_wait_sleep,
			&size, TID_DWORD, TRUE);
	assert(status == DB_SUCCESS);

	printf("Ring buffer wait sleep %d ms\n", test_rb_wait_sleep);
}

#include "msystem.h"

int test_run_number = 0;
int test_rb_wait_count = 0;
int test_event_count = 0;
int test_rbh = 0;

int test_thread(void *param) {
	int status;
	EVENT_HEADER *pevent;
	void *p;
	EQUIPMENT* eq = &equipment[0];

	/* indicate activity to framework */
	signal_readout_thread_active(test_rbh, 1);

	while (!stop_all_threads) {
		if (test_run_number == 0) {
			// no run, wait
			ss_sleep(10);
			continue;
		}

		/* obtain buffer space */
		status = rb_get_wp(get_event_rbh(test_rbh), &p, 0);
		if (stop_all_threads)
			break;
		if (status == DB_TIMEOUT) {
			test_rb_wait_count++;
			//printf("readout_thread: Ring buffer is full, waiting for space!\n");
			if (test_rb_wait_sleep)
				ss_sleep(test_rb_wait_sleep);
			continue;
		}
		if (status != DB_SUCCESS) {
			// catastrophic failure of ring buffer?
			break;
		}

		if (eq->info.period) {
			ss_sleep(eq->info.period);
		}

		if (1 /*readout_enabled()*/) {

			/* check for new event */
			//source = poll_event(multithread_eq->info.source, multithread_eq->poll_count, FALSE);
			if (1 /*source > 0*/) {

				if (stop_all_threads)
					break;

				pevent = (EVENT_HEADER *) p;

				/* compose MIDAS event header */
				pevent->event_id = eq->info.event_id;
				pevent->trigger_mask = eq->info.trigger_mask;
				pevent->data_size = 0;
				pevent->time_stamp = ss_time();
				pevent->serial_number = eq->serial_number++;

				/* call user readout routine */
				pevent->data_size = eq->readout((char *) (pevent + 1), 0);

				/* check event size */
				if (pevent->data_size + sizeof(EVENT_HEADER)
						> (DWORD) max_event_size) {
					cm_msg(MERROR, "readout_thread",
							"Event size %ld larger than maximum size %d",
							(long) (pevent->data_size + sizeof(EVENT_HEADER)),
							max_event_size);
					assert (FALSE);
				}

				if (pevent->data_size > 0) {
					/* put event into ring buffer */
					rb_increment_wp(get_event_rbh(test_rbh),
							sizeof(EVENT_HEADER) + pevent->data_size);
				} else
					eq->serial_number--;
			}

		} else
			// readout_enabled
			ss_sleep(10);
	}

	signal_readout_thread_active(test_rbh, 0);

	return 0;
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

	int fail = 0;
	int status;
	int size;

	size = sizeof(fail);
	status = db_get_value(hDB, hSet, "fail_begin_of_run", &fail, &size, TID_INT,
			TRUE);
	assert(status == DB_SUCCESS);

	if (fail) {
		printf("fail_begin_of_run: returning error status %d\n", fail);
		return fail;
	}

	configure();

	int s = 0;
	size = sizeof(s);
	status = db_get_value(hDB, hSet, "sleep_begin_of_run", &s, &size, TID_INT,
			TRUE);
	assert(status == DB_SUCCESS);

	if (s) {
		printf("sleep_begin_of_run: calling ss_sleep(%d)\n", s);
		ss_sleep(s);
	}

	test_event_count = 0;
	test_rb_wait_count = 0;
	test_run_number = run_number; // tell thread to start running

	return SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/

INT end_of_run(INT run_number, char *error) {
	std::cout << "end_of_run(" << run_number << ", " << error << ")"
			<< std::endl;

	int fail = 0;
	int status;
	int size;

	size = sizeof(fail);
	status = db_get_value(hDB, hSet, "fail_end_of_run", &fail, &size, TID_INT,
			TRUE);
	assert(status == DB_SUCCESS);

	if (fail) {
		printf("fail_end_of_run: returning error status %d\n", fail);
		return fail;
	}

	test_run_number = 0; // tell thread to stop running

	int s = 0;
	size = sizeof(s);
	status = db_get_value(hDB, hSet, "sleep_end_of_run", &s, &size, TID_INT,
			TRUE);
	assert(status == DB_SUCCESS);

	if (s) {
		printf("sleep_end_of_run: calling ss_sleep(%d)\n", s);
		ss_sleep(s);
	}

	printf("test_event_count: %d events sent, ring buffer wait count %d\n",
			test_event_count, test_rb_wait_count);

	return SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/

INT pause_run(INT run_number, char *error) {
	std::cout << "pause_run(" << run_number << ", " << error << ")"
			<< std::endl;

	int fail = 0;
	int status;
	int size;

	size = sizeof(fail);
	status = db_get_value(hDB, hSet, "fail_pause_run", &fail, &size, TID_INT,
			TRUE);
	assert(status == DB_SUCCESS);

	if (fail) {
		printf("fail_pause_run: returning error status %d\n", fail);
		return fail;
	}

	test_run_number = 0; // tell thread to stop running
	return SUCCESS;
}

/*-- Resume Run ----------------------------------------------------*/

INT resume_run(INT run_number, char *error) {
	std::cout << "resume_run(" << run_number << ", " << error << ")"
			<< std::endl;

	int fail = 0;
	int status;
	int size;

	size = sizeof(fail);
	status = db_get_value(hDB, hSet, "fail_resume_run", &fail, &size, TID_INT,
			TRUE);
	assert(status == DB_SUCCESS);

	if (fail) {
		printf("fail_resume_run: returning error status %d\n", fail);
		return fail;
	}

	test_run_number = run_number; // tell thread to start running
	return SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/

INT frontend_loop() {
	/* if frontend_call_loop is true, this routine gets called when
	 the frontend is idle or once between every event */
	return SUCCESS;
}

/*------------------------------------------------------------------*/

/********************************************************************\
  
 Readout routines for different events

 \********************************************************************/

INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
 is available. If test equals TRUE, don't return. The test
 flag is used to time the polling */
{
	if (test) {
		ss_sleep(count);
	}
	return (0);
}

/*-- Interrupt configuration ---------------------------------------*/

INT interrupt_configure(INT cmd, INT source, PTYPE adr) {
	std::cout << "interrupt_configure(" << cmd << ", " << source << ")"
			<< std::endl;

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

/*-- Event readout -------------------------------------------------*/

int read_test_event(char *pevent, int off) {
	bk_init32(pevent);

	char* pdata8;

	bk_create(pevent, "TEST", TID_BYTE, (void**) &pdata8);
	fe::BankType *bank = (fe::BankType*) pdata8;
	bank->headerSize = offsetof(fe::BankType, data);
	std::cout << "bank->headerSize=" << bank->headerSize << std::endl;
	bank->dataType = fe::DataType::WaveForm16bitVer1;
	bank->device = fe::Device::CaenV1720E;
	bank->customHeader.waveForm16BitVer1.sampleTime = 1000000 / 250;

	pdata8 += event_size;

	bk_close(pevent, pdata8);

	test_event_count++;

	return bk_size(pevent);
}

int read_slow_event(char *pevent, int off) {
	bk_init32(pevent);

	float* pdataf;

	bk_create(pevent, "SLOW", TID_FLOAT, (void**) &pdataf);

	time_t t = time(NULL);

	pdataf[0] = 100.0 * sin(M_PI * t / 60);

	printf("time %d, data %f\n", (int) t, pdataf[0]);

	bk_close(pevent, pdataf + 1);

	return bk_size(pevent);
}

int read_random_event(char *pevent, int off) {
	if (drand48() < 0.5)
		bk_init(pevent);
	else
		bk_init32(pevent);

	int nbank = 1 + 8 * drand48();

	for (int i = nbank; i >= 0; i--) {
		char name[5];
		name[0] = 'R';
		name[1] = 'N';
		name[2] = 'D';
		name[3] = '0' + i;
		name[4] = 0;

		int tid = 1 + (TID_LAST - 1) * drand48();

		int size = 100 * drand48();

		char* ptr;
		bk_create(pevent, name, tid, (void**) &ptr);

		for (int j = 0; j < size; j++)
			ptr[j] = i;

		bk_close(pevent, ptr + size);
	}

	return bk_size(pevent);
}

INT connectToDevice(HNDLE const hDB, HNDLE const hSet) {

	int32_t const linkNum = odb::getValueInt32(hDB, hSet, "link_num", TRUE,
			defaults::linkNum);
	int32_t const conetNode = odb::getValueInt32(hDB, hSet, "conet_node", TRUE,
			defaults::conetNode);
	uint32_t const vmeBaseAddr = odb::getValueUInt32(hDB, hSet, "vme_base_addr",
			TRUE, defaults::vmeBaseAddr);

	cm_msg(MDEBUG, frontend_name, "Connecting to device");
	globals::boardInfo.Channels = 8;
	cm_msg(MINFO, frontend_name, "Connected to device");

	return DB_SUCCESS;

}

std::string channelKey(decltype(globals::boardInfo.Channels) const channelNo,
		char const * const keyName) {

	return std::string("channel_") + std::to_string(channelNo) + "_" + keyName;

}

INT configureDevice(HNDLE const hDB, HNDLE const hSet) {

	cm_msg(MDEBUG, frontend_name, "Configuring device");

	uint32_t channelMask = 0x0000;
	uint32_t const recordLength = odb::getValueUInt32(hDB, hSet,
			"waveform_length", TRUE, defaults::recordLength);
	uint32_t const preTriggerLength = odb::getValueUInt32(hDB, hSet,
			"pre_trigger_length", TRUE, defaults::preTriggerLength);
	std::string const triggerMode = odb::getValueString(hDB, hSet,
			"trigger_mode", TRUE, defaults::triggerMode);
	uint8_t const triggerChannel = odb::getValueUInt8(hDB, hSet,
			"trigger_channel", TRUE, defaults::triggerChannel);
	uint32_t const triggerThreshold = odb::getValueUInt32(hDB, hSet,
			"trigger_threshold", TRUE, defaults::triggerThreshold);
	bool const triggerRaisingPolarity = odb::getValueBool(hDB, hSet,
			"trigger_raising_polarity", TRUE, defaults::triggerRaisingPolarity);

	for (decltype(globals::boardInfo.Channels) i = 0;
			i < globals::boardInfo.Channels; i++) {

		bool const enabled = odb::getValueBool(hDB, hSet,
				channelKey(i, "enabled"), TRUE, defaults::channel::enabled);
		if (enabled) {
			channelMask |= 0x0001 << i;
		}

		uint32_t const dcOffset = odb::getValueUInt32(hDB, hSet,
				channelKey(i, "dc_offset"), TRUE, defaults::channel::dcOffset);

	}

	cm_msg(MDEBUG, frontend_name, "Device is successfully configured");

	return DB_SUCCESS;

}

INT frontend_init() {
	std::cout << "frontend_init()" << std::endl;

	int status;

	// create subtree
	odb::getValueInt32(hDB, 0, "/equipment/" EQUIP_NAME "/Settings/link_num",
			TRUE, defaults::linkNum);

	// save reference to settings tree
	status = db_find_key(hDB, 0, "/equipment/" EQUIP_NAME "/Settings", &hSet);
	assert(status == DB_SUCCESS);

	status = connectToDevice(hDB, hSet);
	assert(status == DB_SUCCESS);

	status = configureDevice(hDB, hSet);
	assert(status == DB_SUCCESS);

#ifdef RPC_JRPC
	status = cm_register_function(RPC_JRPC, rpc_callback);
	assert(status == SUCCESS);
#endif

	create_event_rb(test_rbh);
	ss_thread_create(test_thread, 0);

//cm_set_watchdog_params (FALSE, 0);

//set_rate_period(1000);

	configure();

	if (false) {
		cm_msg(MERROR, frontend_name,
				"Error initializing the front-end, status %d", status);
		return 1;
	}

	return SUCCESS;
}
