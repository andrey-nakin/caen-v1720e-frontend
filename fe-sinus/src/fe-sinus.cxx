#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstddef>
#include <stdlib.h>
#include <cstring>
#include <memory>
#include <sstream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <ctime>
#include <cmath>
#include <midas.h>

#include <midas/odb.hxx>
#include <frontend/types.hxx>

#include "defaults.hxx"

#define EQUIP_NAME "sinus"
constexpr int EVID = 1;
constexpr auto PI = 3.1415926536;

namespace globals {

static uint8_t numOfChannels = 1;
static uint32_t recordLength = 0;
static midas_thread_t readoutThread;
static std::atomic_bool acquisitionIsOn(false);
static std::mutex readingMutex;
static uint64_t runStartTime;
static uint32_t eventCounter = 0;
static uint32_t channelMask = 0x0000;
static uint32_t dFrequency;

static std::vector<bool> enabledChannels;
static std::vector<uint16_t> dcOffsets;
static std::vector<uint16_t> amplitudes;
static std::vector<uint32_t> frequencies;
static std::vector<int32_t> phases;

}

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

int readEvent(char *pevent, int off);

/*-- Equipment list ------------------------------------------------*/

EQUIPMENT equipment[] = { { EQUIP_NAME, { EVID, (1 << EVID), /* event ID, trigger mask */
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
"", "", "" }, readEvent, /* readout routine */
}, { "" } };

#ifndef NEED_NO_EXTERN_C
}
#endif

extern "C" {
void set_rate_period(int ms);
}

int test_rb_wait_sleep = 1;

#include "msystem.h"

int test_rb_wait_count = 0;
int test_rbh = 0;

int test_thread(void * /* param */) {
	int status;
	EVENT_HEADER *pevent;
	void *p;
	EQUIPMENT* eq = &equipment[0];

	/* indicate activity to framework */
	signal_readout_thread_active(test_rbh, 1);

	while (!stop_all_threads) {
		if (!globals::acquisitionIsOn.load(std::memory_order_relaxed)) {
			// no run, wait
			ss_sleep(1);
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
			ss_sleep(1);
	}

	signal_readout_thread_active(test_rbh, 0);

	return 0;
}

INT poll_event(INT /* source */, INT count, BOOL test) {

	if (test) {
		ss_sleep(count);
	}
	return (0);

}

INT interrupt_configure(INT /* cmd */, INT /* source */, PTYPE /* adr */) {
	return SUCCESS;
}

template<typename T>
static std::string toString(T const v, std::size_t const len) {

	auto s = std::to_string(v);
	while (s.size() < len) {
		s = "0" + s;
	}
	return s;

}

static std::string channelKey(uint8_t const channelNo,
		char const * const keyName) {

	return std::string("channel_") + std::to_string(channelNo) + "_" + keyName;

}

static HNDLE getSettingsKey() {

	return odb::findKey(hDB, 0, "/equipment/" EQUIP_NAME "/Settings");

}

static void configure() {

	auto const hSet = getSettingsKey();

	globals::numOfChannels = odb::getValueUInt8(hDB, hSet, "num_of_channels",
			TRUE, defaults::numOfChannels);

	globals::enabledChannels.resize(globals::numOfChannels);
	globals::dcOffsets.resize(globals::numOfChannels);
	globals::frequencies.resize(globals::numOfChannels);
	globals::amplitudes.resize(globals::numOfChannels);
	globals::phases.resize(globals::numOfChannels);

	globals::dFrequency = odb::getValueUInt32(hDB, hSet, "discrete_frequency",
			TRUE, defaults::dFrequency);
	globals::recordLength = odb::getValueUInt32(hDB, hSet, "waveform_length",
			TRUE, defaults::recordLength);

	globals::channelMask = 0x0000;
	for (uint8_t i = 0; i < globals::numOfChannels; i++) {
		globals::enabledChannels[i] = odb::getValueBool(hDB, hSet,
				channelKey(i, "enabled"), TRUE, defaults::channel::enabled);
		globals::dcOffsets[i] = odb::getValueUInt16(hDB, hSet,
				channelKey(i, "dc_offset"), TRUE, defaults::channel::dcOffset);
		globals::frequencies[i] = odb::getValueUInt32(hDB, hSet,
				channelKey(i, "frequency"), TRUE, defaults::channel::frequency);
		globals::amplitudes[i] = odb::getValueUInt16(hDB, hSet,
				channelKey(i, "amplitude"), TRUE, defaults::channel::amplitude);
		globals::phases[i] = odb::getValueInt32(hDB, hSet,
				channelKey(i, "phase"), TRUE, defaults::channel::phase);

		if (globals::enabledChannels[i]) {
			globals::channelMask |= 0x0001 << i;
		}
	}

}

static uint64_t nanoTime() {

	std::chrono::time_point < std::chrono::system_clock > now =
			std::chrono::system_clock::now();
	auto duration = now.time_since_epoch();
	auto nanoseconds = std::chrono::duration_cast < std::chrono::nanoseconds
			> (duration);
	return nanoseconds.count();

}

static void startAcquisition() {

	globals::runStartTime = nanoTime();
	globals::eventCounter = 0;
	globals::acquisitionIsOn.store(true);

}

static void stopAcquisition() {

	globals::acquisitionIsOn.store(false);

	std::lock_guard < std::mutex > lock(globals::readingMutex);

}

INT frontend_init() {
	int status = SUCCESS;

	try {
		// create subtree
		odb::getValueUInt8(hDB, 0,
				"/equipment/" EQUIP_NAME "/Settings/num_of_channels", TRUE,
				defaults::numOfChannels);

		create_event_rb(test_rbh);
		globals::readoutThread = ss_thread_create(test_thread, 0);

		configure();

	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	}

	return status;
}

INT frontend_exit() {

	int status = SUCCESS;

	try {
		stopAcquisition();
		ss_thread_kill(globals::readoutThread);
	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	}

	return status;

}

INT begin_of_run(INT /* run_number */, char * /* error */) {

	int status = SUCCESS;

	try {
		configure();

		startAcquisition();

		test_rb_wait_count = 0;

	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	}

	return status;

}

INT end_of_run(INT /* run_number */, char * /* error */) {

	int status = SUCCESS;

	try {
		stopAcquisition();
	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	}

	return status;

}

INT pause_run(INT /* run_number */, char * /*error */) {

	int status = SUCCESS;

	try {
		stopAcquisition();
	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	}

	return status;

}

INT resume_run(INT /* run_number */, char * /*error */) {

	int status = SUCCESS;

	try {
		startAcquisition();
	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	}

	return status;

}

INT frontend_loop() {

	return SUCCESS;

}

static int buildEvent(char * const pevent) {

	bk_init32(pevent);

	{
		// store general information
		uint8_t* pdata;
		bk_create(pevent, "INFO", TID_DWORD, (void**) &pdata);
		fe::InfoBank* info = (fe::InfoBank*) pdata;
		info->dataType = fe::DataType::WaveForm16bitVer1;
		info->deviceType = fe::DeviceType::CaenV1720E;
		info->boardId = 0;
		info->channelMask = globals::channelMask;
		info->eventCounter = ++globals::eventCounter;
		info->timeStamp = nanoTime();
		info->recordLength = globals::recordLength;
		info->preTriggerLength = 0;
		bk_close(pevent, pdata + sizeof(*info));
	}

	{
		// store channel DC offset
		uint16_t* pdata;
		bk_create(pevent, "CHDC", TID_WORD, (void**) &pdata);
		for (uint8_t i = 0; i < globals::numOfChannels; i++) {
			*pdata++ = globals::dcOffsets[i];
		}
		bk_close(pevent, pdata);
	}

	// store wave forms
	auto const t = nanoTime() - globals::runStartTime;
	for (uint8_t i = 0; i < globals::numOfChannels; i++) {
		if (globals::enabledChannels[i]) {
			auto const dt = 1.0e9 / globals::frequencies[i];

			if (globals::recordLength > 0) {
				std::string const name = "WF" + toString(i, 2);
				uint16_t* pdata;
				bk_create(pevent, name.c_str(), TID_WORD, (void**) &pdata);

				for (uint32_t j = 0; j < globals::recordLength; j++) {
					auto const ns = static_cast<int64_t>(j) * 1000000000
							/ globals::dFrequency + t - globals::phases[i];
					auto const v = std::sin(2.0 * PI * ns / dt)
							* globals::amplitudes[i] + globals::dcOffsets[i];
					*pdata++ = v < 0.0 ? 0 :
								v > 0xffff ? 0xffff : static_cast<uint16_t>(v);
				}

				bk_close(pevent, pdata);
			}
		}
	}

	return bk_size(pevent);

}

int readEvent(char * const pevent, const int /* off */) {

	std::lock_guard < std::mutex > lock(globals::readingMutex);
	int result;

	if (globals::acquisitionIsOn.load(std::memory_order_relaxed)) {
		try {
			result = buildEvent(pevent);
		} catch (midas::Exception& ex) {
			result = 0;
		}
	} else {
		result = 0;
	}

	return result;

}
