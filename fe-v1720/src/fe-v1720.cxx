#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstddef>
#include <stdlib.h>
#include <cstring>
#include <memory>
#include <sstream>
#include <midas.h>

#include <midas/odb.hxx>
#include <frontend/types.hxx>
#include <frontend/locker.hxx>
#include <caen/handle.hxx>
#include <caen/error-holder.hxx>
#include <caen/readout-buffer.hxx>
#include <caen/event.hxx>
#include <caen/exception.hxx>
#include <caen/v1720.hxx>

#include "defaults.hxx"

#define EQUIP_NAME "v1720"
constexpr int32_t FIRST_EVENT = 0;
constexpr int EVID = 1;

namespace globals {

static CAEN_DGTZ_BoardInfo_t boardInfo;
static uint32_t recordLength = 0;
static uint32_t preTriggerLength = 0;
static std::vector<bool> enabledChannels;
static std::vector<uint16_t> dcOffsets;
static std::vector<uint16_t> sample;
static std::unique_ptr<caen::Handle> hDevice;
static std::unique_ptr<caen::ReadoutBuffer> roBuffer;
static std::unique_ptr<caen::Event> event;
static uint32_t eventCounter;
static midas_thread_t readoutThread;
static std::atomic_bool isReading(false);
static std::atomic_bool acquisitionStarted(false);

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

int test_thread(void *param) {
	int status;
	EVENT_HEADER *pevent;
	void *p;
	EQUIPMENT* eq = &equipment[0];

	/* indicate activity to framework */
	signal_readout_thread_active(test_rbh, 1);

	while (!stop_all_threads) {
		if (!globals::acquisitionStarted.load(std::memory_order_acquire)) {
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

INT interrupt_configure(INT cmd, INT source, PTYPE adr) {
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

template<typename T>
static std::string toString(T const v, std::size_t const len) {

	auto s = std::to_string(v);
	while (s.size() < len) {
		s = "0" + s;
	}
	return s;

}

static std::string channelKey(unsigned const channelNo,
		char const * const keyName) {

	return std::string("channel_") + std::to_string(channelNo) + "_" + keyName;

}

static HNDLE getSettingsKey() {

	return odb::findKey(hDB, 0, "/equipment/" EQUIP_NAME "/Settings");

}

static INT handleCaenException(caen::Exception const& ex) {

	INT const status = CM_SET_ERROR;
	cm_msg(MERROR, frontend_name, ex.what(), status);
	return status;

}

static caen::Handle connect() {

	// save reference to settings tree
	auto const hSet = getSettingsKey();

	auto const linkNum = odb::getValueInt32(hDB, hSet, "link_num", TRUE,
			defaults::linkNum);
	auto const conetNode = odb::getValueInt32(hDB, hSet, "conet_node", TRUE,
			defaults::conetNode);
	auto const vmeBaseAddr = odb::getValueUInt32(hDB, hSet, "vme_base_addr",
			TRUE, defaults::vmeBaseAddr);

	caen::Handle result(linkNum, conetNode, vmeBaseAddr);

	return result;

}

static void configure(caen::Handle& hDevice) {

	auto const hSet = getSettingsKey();

	decltype(globals::boardInfo) boardInfo;
	hDevice.hCommand("getting digitizer info",
			[&boardInfo](int handle) {return CAEN_DGTZ_GetInfo(handle, &boardInfo);});
	globals::boardInfo = boardInfo;
	globals::enabledChannels.resize(globals::boardInfo.Channels);
	globals::dcOffsets.resize(globals::boardInfo.Channels);

	hDevice.hCommand("resetting digitizer", CAEN_DGTZ_Reset);

	hDevice.hCommand("setting IO level",
			[](int handle) {return CAEN_DGTZ_SetIOLevel(handle, CAEN_DGTZ_IOLevel_NIM);});

	hDevice.hCommand("setting external trigger input mode",
			[](int handle) {return CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);});

	uint32_t channelMask = 0x0000;
	for (unsigned i = 0; i < globals::boardInfo.Channels; i++) {
		globals::enabledChannels[i] = odb::getValueBool(hDB, hSet,
				channelKey(i, "enabled"), TRUE, defaults::channel::enabled);
		if (globals::enabledChannels[i]) {
			channelMask |= 0x0001 << i;
		}
	}
	hDevice.hCommand("setting channel enable mask",
			[channelMask](int handle) {return CAEN_DGTZ_SetChannelEnableMask(handle, channelMask);});

	hDevice.hCommand("setting run sync mode",
			[](int handle) {return CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled);});

	decltype(globals::recordLength) const recordLength = odb::getValueUInt32(
			hDB, hSet, "waveform_length", TRUE, defaults::recordLength);
	globals::recordLength = recordLength;

	for (unsigned i = 0; i < globals::boardInfo.Channels; i++) {
		hDevice.hCommand("setting record length",
				[recordLength, i](int handle) {return CAEN_DGTZ_SetRecordLength(handle, recordLength, i);});

		globals::dcOffsets[i] = odb::getValueUInt16(hDB, hSet,
				channelKey(i, "dc_offset"), TRUE, defaults::channel::dcOffset);

		decltype(globals::dcOffsets[i]) dcOffset = globals::dcOffsets[i];
		hDevice.hCommand("setting channel DC offset",
				[dcOffset, i](int handle) {return CAEN_DGTZ_SetChannelDCOffset(handle, i, dcOffset);});
	}

	auto const triggerMode = odb::getValueString(hDB, hSet,
			"trigger_mode", TRUE, defaults::triggerMode);

	auto const triggerChannel = odb::getValueUInt8(hDB, hSet,
			"trigger_channel", TRUE, defaults::triggerChannel);
	hDevice.hCommand("setting channel self trigger",
			[triggerChannel](int handle) {return CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, (1 << triggerChannel));});

	auto const triggerThreshold = odb::getValueUInt16(hDB, hSet,
			"trigger_threshold", TRUE, defaults::triggerThreshold);
	hDevice.hCommand("setting channel trigger threshold",
			[triggerChannel, triggerThreshold](int handle) {return CAEN_DGTZ_SetChannelTriggerThreshold(handle, triggerChannel, triggerThreshold);});

	auto const triggerRaisingPolarity = odb::getValueBool(hDB, hSet,
			"trigger_raising_polarity", TRUE, defaults::triggerRaisingPolarity);
	hDevice.hCommand("setting trigger polarity",
			[triggerChannel, triggerRaisingPolarity](int handle) {return CAEN_DGTZ_SetTriggerPolarity(handle, triggerChannel, triggerRaisingPolarity ? CAEN_DGTZ_TriggerOnRisingEdge : CAEN_DGTZ_TriggerOnFallingEdge);});

	hDevice.hCommand("setting max num events",
			[](int handle) {return CAEN_DGTZ_SetMaxNumEventsBLT(handle, 1);});

	hDevice.hCommand("setting acquisition mode",
			[](int handle) {return CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);});

	auto const regData = hDevice.readRegister(caen::v1720::REG_CHANNEL_CONFIG);
	if (regData & caen::v1720::REG_BIT_TRIGGER_OVERLAP) {
		// disable trigger overlap
		hDevice.writeRegister(caen::v1720::REG_CHANNEL_CONFIG,
				regData & ~caen::v1720::REG_BIT_TRIGGER_OVERLAP);
	}

	globals::preTriggerLength = odb::getValueUInt32(hDB, hSet,
			"pre_trigger_length", TRUE, defaults::preTriggerLength);

	hDevice.writeRegister(caen::v1720::REG_POST_TRIGGER,
			(globals::recordLength - globals::preTriggerLength) / 4);

}

static void startAcquisition() {

	globals::roBuffer = std::unique_ptr < caen::ReadoutBuffer
			> (new caen::ReadoutBuffer(*globals::hDevice));

	globals::event = std::unique_ptr < caen::Event
			> (new caen::Event(*globals::hDevice));

	globals::hDevice->hCommand("starting acquisition",
			CAEN_DGTZ_SWStartAcquisition);

	globals::acquisitionStarted.store(true);

}

static void stopAcquisition() {

	globals::acquisitionStarted.store(false);

	// wait until reading completes
	while (globals::isReading.load()) {
		ss_sleep(10);
	}

	globals::hDevice->hCommand("stopping acquisition",
			CAEN_DGTZ_SWStopAcquisition);

	globals::event = nullptr;
	globals::roBuffer = nullptr;

}

INT frontend_init() {
	int status = SUCCESS;

	try {
		// create subtree
		odb::getValueInt32(hDB, 0,
				"/equipment/" EQUIP_NAME "/Settings/link_num", TRUE,
				defaults::linkNum);

		create_event_rb(test_rbh);
		globals::readoutThread = ss_thread_create(test_thread, 0);

		caen::Handle hDevice = connect();
		configure(hDevice);

	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	} catch (caen::Exception& ex) {
		status = handleCaenException(ex);
	}

	return status;
}

INT frontend_exit() {

	int status = SUCCESS;

	try {

		if (globals::hDevice) {
			stopAcquisition();
			globals::hDevice = nullptr;
		}

		ss_thread_kill(globals::readoutThread);

	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	} catch (caen::Exception& ex) {
		status = handleCaenException(ex);
	}

	return status;

}

INT begin_of_run(INT run_number, char *error) {

	int status = SUCCESS;

	try {
		globals::hDevice = std::unique_ptr < caen::Handle
				> (new caen::Handle(connect()));
		configure(*globals::hDevice);

		startAcquisition();

		test_rb_wait_count = 0;

	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	} catch (caen::Exception& ex) {
		status = handleCaenException(ex);
	}

	return status;

}

INT end_of_run(INT run_number, char *error) {

	int status = SUCCESS;

	try {

		if (globals::hDevice) {
			stopAcquisition();
			globals::hDevice = nullptr;
		}

	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	} catch (caen::Exception& ex) {
		status = handleCaenException(ex);
	}

	return status;

}

INT pause_run(INT run_number, char *error) {

	int status = SUCCESS;

	try {

		stopAcquisition();

	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	} catch (caen::Exception& ex) {
		status = handleCaenException(ex);
	}

	return status;

}

INT resume_run(INT run_number, char *error) {

	int status = SUCCESS;

	try {

		startAcquisition();

	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	} catch (caen::Exception& ex) {
		status = handleCaenException(ex);
	}

	return status;

}

INT frontend_loop() {

	return SUCCESS;

}

int readEvent(char *pevent, int off) {

	fe::Locker locker(globals::isReading);
	if (!globals::acquisitionStarted.load(std::memory_order_acquire)) {
		return 0;
	}

	int result;

	try {
		uint32_t const dataSize = globals::roBuffer->readData();

		uint32_t const numEvents = globals::roBuffer->getNumEvents(dataSize);

		if (!numEvents) {
			return 0;	//	no events
		}

		std::pair<CAEN_DGTZ_EventInfo_t, char*> evt =
				globals::roBuffer->getEventInfo(FIRST_EVENT);
		CAEN_DGTZ_EventInfo_t const& eventInfo = evt.first;

		if (eventInfo.EventCounter > globals::eventCounter + 1) {
			cm_msg(MERROR, frontend_name, "%u event(s) has been lost",
					eventInfo.EventCounter - globals::eventCounter - 1);
		}
		globals::eventCounter = eventInfo.EventCounter;

		globals::event->decode(evt.second);

		bk_init32(pevent);

		{
			// store general information
			uint8_t* pdata;
			bk_create(pevent, "INFO", TID_DWORD, (void**) &pdata);
			fe::InfoBank* info = (fe::InfoBank*) pdata;
			info->dataType = fe::DataType::WaveForm16bitVer1;
			info->device = fe::Device::CaenV1720E;
			info->recordLength = globals::recordLength;
			info->preTriggerLength = globals::preTriggerLength;
			info->timeStamp = 0;	// TODO
			bk_close(pevent, pdata + sizeof(*info));
		}

		{
			// store channel enabled status
			uint8_t* pdata;
			bk_create(pevent, "CHEN", TID_BYTE, (void**) &pdata);
			for (decltype(globals::boardInfo.Channels) i = 0;
					i < globals::boardInfo.Channels; i++) {
				*pdata++ = globals::enabledChannels[i] ? 1 : 0;
			}
			bk_close(pevent, pdata);
		}

		{
			// store channel DC offset
			uint16_t* pdata;
			bk_create(pevent, "CHDC", TID_WORD, (void**) &pdata);
			for (decltype(globals::boardInfo.Channels) i = 0;
					i < globals::boardInfo.Channels; i++) {
				*pdata++ = globals::dcOffsets[i];
			}
			bk_close(pevent, pdata);
		}

		// store wave forms
		for (unsigned i = 0; i < globals::boardInfo.Channels; i++) {
			if (eventInfo.ChannelMask & (0x0001 << i)) {
				uint32_t const numOfSamples = globals::event->evt()->ChSize[i];
				uint16_t const *samples = globals::event->evt()->DataChannel[i];
				uint32_t const dataSize = numOfSamples * sizeof(*samples);

				std::string const name = "WF" + toString(i, 2);
				uint8_t* pdata;
				bk_create(pevent, name.c_str(), TID_WORD, (void**) &pdata);
				std::memcpy(pdata, samples, dataSize);
				bk_close(pevent, pdata + dataSize);
			}
		}

		result = bk_size(pevent);

	} catch (midas::Exception& ex) {
		result = 0;
	} catch (caen::Exception& ex) {
		handleCaenException(ex);
		result = 0;
	}

	return result;

}
