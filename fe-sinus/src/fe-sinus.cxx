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
#include <util/types.hxx>
#include <util/V1720InfoRawData.hxx>
#include <util/TDcOffsetRawData.hxx>
#include <util/TWaveFormRawData.hxx>
#include <util/FrontEndUtils.hxx>

#include "defaults.hxx"

#define EQUIP_NAME "sinus"
constexpr int EVID = 1;
constexpr auto PI = 3.1415926536;
constexpr std::size_t NUM_OF_CHANNELS = 8;

namespace glob {

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
INT get_frontend_index();

/*-- Equipment list ------------------------------------------------*/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

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

#pragma GCC diagnostic pop

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
		if (!glob::acquisitionIsOn.load(std::memory_order_relaxed)) {
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

static void configure() {

	auto const hSet = util::FrontEndUtils::settingsKey(EQUIP_NAME);

	glob::dFrequency = odb::getValueUInt32(hDB, hSet, "discrete_frequency",
			defaults::dFrequency, true);
	glob::recordLength = odb::getValueUInt32(hDB, hSet, "waveform_length",
			defaults::recordLength, true);

	glob::enabledChannels = odb::getValueBoolV(hDB, hSet, "channel_enabled",
			NUM_OF_CHANNELS, true, true);
	glob::dcOffsets = odb::getValueUInt16V(hDB, hSet, "channel_dc_offset",
			NUM_OF_CHANNELS, defaults::channel::dcOffset, true);
	glob::frequencies = odb::getValueUInt32V(hDB, hSet, "channel_frequency",
			NUM_OF_CHANNELS, defaults::channel::frequency, true);
	glob::amplitudes = odb::getValueUInt16V(hDB, hSet, "channel_amplitude",
			NUM_OF_CHANNELS, defaults::channel::amplitude, true);
	glob::phases = odb::getValueInt32V(hDB, hSet, "channel_phase",
			NUM_OF_CHANNELS, defaults::channel::phase, true);

	glob::channelMask = 0x0000;
	for (uint8_t i = 0; i < NUM_OF_CHANNELS; i++) {
		if (glob::enabledChannels[i]) {
			glob::channelMask |= 0x0001 << i;
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

	glob::runStartTime = nanoTime();
	glob::eventCounter = 0;
	glob::acquisitionIsOn.store(true);

}

static void stopAcquisition() {

	glob::acquisitionIsOn.store(false);

	std::lock_guard < std::mutex > lock(glob::readingMutex);

}

INT frontend_init() {

	return util::FrontEndUtils::command([]() {

		// create subtree
			odb::getValueUInt32(hDB, 0,
					util::FrontEndUtils::settingsKeyName(EQUIP_NAME,
							"waveform_length"), defaults::recordLength, true);

			create_event_rb(test_rbh);
			glob::readoutThread = ss_thread_create(test_thread, 0);

			configure();

		});

}

INT frontend_exit() {

	return util::FrontEndUtils::command([]() {

		stopAcquisition();
		ss_thread_kill(glob::readoutThread);

	});

}

INT begin_of_run(INT /* run_number */, char * /* error */) {

	return util::FrontEndUtils::command([]() {

		configure();

		startAcquisition();

		test_rb_wait_count = 0;

	});

}

INT end_of_run(INT /* run_number */, char * /* error */) {

	return util::FrontEndUtils::command([]() {

		stopAcquisition();

	});

}

INT pause_run(INT /* run_number */, char * /*error */) {

	return util::FrontEndUtils::command([]() {

		stopAcquisition();

	});

}

INT resume_run(INT /* run_number */, char * /*error */) {

	return util::FrontEndUtils::command([]() {

		startAcquisition();

	});

}

INT frontend_loop() {

	return SUCCESS;

}

static int buildEvent(char * const pevent) {

	bk_init32(pevent);

	{
		// store general information
		uint8_t* pdata;
		bk_create(pevent, util::V1720InfoRawData::bankName(), TID_DWORD,
				(void**) &pdata);
		util::InfoBank* info = (util::InfoBank*) pdata;
		info->boardId = 0;
		info->channelMask = glob::channelMask;
		info->eventCounter = ++glob::eventCounter;
		auto const t = nanoTime();
		info->timeStampLo = t & 0xffffffff;
		info->timeStampHi = t >> 32;
		info->timeStampHi = t >> 32;
		info->frontendIndex = get_frontend_index();
		bk_close(pevent, pdata + sizeof(*info));
	}

	{
		// store channel DC offset
		uint16_t* pdata;
		bk_create(pevent, util::TDcOffsetRawData::BANK_NAME, TID_WORD,
				(void**) &pdata);
		for (uint8_t i = 0; i < NUM_OF_CHANNELS; i++) {
			*pdata++ = glob::dcOffsets[i];
		}
		bk_close(pevent, pdata);
	}

	// store wave forms
	auto const t = nanoTime() - glob::runStartTime;
	for (uint8_t i = 0; i < NUM_OF_CHANNELS; i++) {
		if (glob::enabledChannels[i]) {
			auto const dt = 1.0e9 / glob::frequencies[i];

			if (glob::recordLength > 0) {
				uint16_t* pdata;
				bk_create(pevent, util::TWaveFormRawData::bankName(i), TID_WORD,
						(void**) &pdata);

				for (uint32_t j = 0; j < glob::recordLength; j++) {
					auto const ns = static_cast<int64_t>(j) * 1000000000
							/ glob::dFrequency + t - glob::phases[i];
					auto const v = std::sin(2.0 * PI * ns / dt)
							* glob::amplitudes[i] + 2048;
					*pdata++ = v < 0.0 ? 0 :
								v > 4095 ? 4095 : static_cast<uint16_t>(v);
				}

				bk_close(pevent, pdata);
			}
		}
	}

	return bk_size(pevent);

}

int readEvent(char * const pevent, const int /* off */) {

	std::lock_guard < std::mutex > lock(glob::readingMutex);
	int result;

	if (glob::acquisitionIsOn.load(std::memory_order_relaxed)) {
		result = util::FrontEndUtils::commandR([pevent]() {

			return buildEvent(pevent);

		});
	} else {
		result = 0;
	}

	return result;

}
