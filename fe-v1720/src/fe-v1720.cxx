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
#include <caen/handle.hxx>
#include <caen/error-holder.hxx>
#include <caen/readout-buffer.hxx>
#include <caen/event.hxx>
#include <caen/exception.hxx>
#include <caen/v1720.hxx>

#include "defaults.hxx"

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

}

#define EQUIP_NAME "v1720"
#define EVID 1

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

EQUIPMENT equipment[] = { { EQUIP_NAME, {
EVID, (1 << EVID), /* event ID, trigger mask */
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

int test_run_number = 0;
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


/********************************************************************\
  
 Readout routines for different events

 \********************************************************************/

INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
 is available. If test equals TRUE, don't return. The test
 flag is used to time the polling */
{

	std::cout << "poll_event(" << source << ", " << count << ", " << test << ")" << std::endl;

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

template<typename T>
static std::string toString(T const v, std::size_t const len) {

	std::string s = std::to_string(v);
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
	HNDLE const hSet = getSettingsKey();

	int32_t const linkNum = odb::getValueInt32(hDB, hSet, "link_num", TRUE,
			defaults::linkNum);
	int32_t const conetNode = odb::getValueInt32(hDB, hSet, "conet_node", TRUE,
			defaults::conetNode);
	uint32_t const vmeBaseAddr = odb::getValueUInt32(hDB, hSet, "vme_base_addr",
			TRUE, defaults::vmeBaseAddr);

	cm_msg(MDEBUG, frontend_name, "Connecting to device");

	caen::Handle result(linkNum, conetNode, vmeBaseAddr);

	cm_msg(MINFO, frontend_name, "Connected to device");

	return result;

}

static void configure(caen::Handle& hDevice) {

	cm_msg(MDEBUG, frontend_name, "Configuring device");

	HNDLE const hSet = getSettingsKey();
	uint32_t regData;

	decltype(globals::boardInfo) boardInfo;
	hDevice.command(
			"getting digitizer info",
			[&boardInfo](auto handle) { return CAEN_DGTZ_GetInfo(handle, &boardInfo); }
	);
	globals::boardInfo = boardInfo;
	globals::enabledChannels.resize(globals::boardInfo.Channels);
	globals::dcOffsets.resize(globals::boardInfo.Channels);

	hDevice.command(
			"resetting digitizer",
			[](auto handle) { return CAEN_DGTZ_Reset(handle); }
	);

	hDevice.command(
			"setting IO level",
			[](auto handle) { return CAEN_DGTZ_SetIOLevel(handle, CAEN_DGTZ_IOLevel_NIM); }
	);

	hDevice.command(
			"setting external trigger input mode",
			[](auto handle) { return CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY); }
	);

	uint32_t channelMask = 0x0000;
	for (unsigned i = 0; i < globals::boardInfo.Channels; i++) {
		globals::enabledChannels[i] = odb::getValueBool(hDB, hSet,
				channelKey(i, "enabled"), TRUE, defaults::channel::enabled);
		if (globals::enabledChannels[i]) {
			channelMask |= 0x0001 << i;
		}
	}
	hDevice.command(
			"setting channel enable mask",
			[channelMask](auto handle) { return CAEN_DGTZ_SetChannelEnableMask(handle, channelMask); }
	);

	hDevice.command(
			"setting run sync mode",
			[](auto handle) { return CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled); }
	);

	decltype(globals::recordLength) const recordLength = odb::getValueUInt32(hDB, hSet, "waveform_length",
			TRUE, defaults::recordLength);
	globals::recordLength = recordLength;

	for (unsigned i = 0; i < globals::boardInfo.Channels; i++) {
		hDevice.command(
				"setting record length",
				[recordLength, i](auto handle) { return CAEN_DGTZ_SetRecordLength(handle, recordLength, i); }
		);

		globals::dcOffsets[i] = odb::getValueUInt16(hDB, hSet,
				channelKey(i, "dc_offset"), TRUE, defaults::channel::dcOffset);

		decltype(globals::dcOffsets[i]) dcOffset = globals::dcOffsets[i];
		hDevice.command(
				"setting channel DC offset",
				[dcOffset, i](auto handle) { return CAEN_DGTZ_SetChannelDCOffset(handle, i, dcOffset); }
		);
	}

	uint8_t const triggerChannel = odb::getValueUInt8(hDB, hSet,
			"trigger_channel", TRUE, defaults::triggerChannel);
	hDevice.command(
			"setting channel self trigger",
			[triggerChannel](auto handle) { return CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, (1 << triggerChannel)); }
	);

	uint16_t const triggerThreshold = odb::getValueUInt16(hDB, hSet,
			"trigger_threshold", TRUE, defaults::triggerThreshold);
	hDevice.command(
			"setting channel trigger threshold",
			[triggerChannel, triggerThreshold](auto handle) { return CAEN_DGTZ_SetChannelTriggerThreshold(handle, triggerChannel, triggerThreshold); }
	);

	bool const triggerRaisingPolarity = odb::getValueBool(hDB, hSet,
			"trigger_raising_polarity", TRUE, defaults::triggerRaisingPolarity);
	hDevice.command(
			"setting trigger polarity",
			[triggerChannel, triggerRaisingPolarity](auto handle) { return CAEN_DGTZ_SetTriggerPolarity(handle, triggerChannel, triggerRaisingPolarity ? CAEN_DGTZ_TriggerOnRisingEdge : CAEN_DGTZ_TriggerOnFallingEdge); }
	);

	hDevice.command(
			"setting max num events",
			[](auto handle) { return CAEN_DGTZ_SetMaxNumEventsBLT(handle, 1); }
	);

	hDevice.command(
			"setting acquisition mode",
			[](auto handle) { return CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED); }
	);

	regData = hDevice.readRegister(caen::v1720::REG_CHANNEL_CONFIG);
	if (regData & caen::v1720::REG_BIT_TRIGGER_OVERLAP) {
		// disable trigger overlap
		hDevice.writeRegister(caen::v1720::REG_CHANNEL_CONFIG, regData & ~caen::v1720::REG_BIT_TRIGGER_OVERLAP);
	}

	globals::preTriggerLength = odb::getValueUInt32(hDB, hSet,
			"pre_trigger_length", TRUE, defaults::preTriggerLength);

	hDevice.writeRegister(caen::v1720::REG_POST_TRIGGER, (globals::recordLength - globals::preTriggerLength) / 4);

	std::string const triggerMode = odb::getValueString(hDB, hSet,
			"trigger_mode", TRUE, defaults::triggerMode);

	cm_msg(MDEBUG, frontend_name, "Device is successfully configured");

}

static void startAcquisition() {

	globals::roBuffer = std::make_unique<caen::ReadoutBuffer>(*globals::hDevice);

	globals::event = std::make_unique<caen::Event>(*globals::hDevice);

	globals::hDevice->command(
			"starting acquisition",
			[](auto handle) { return CAEN_DGTZ_SWStartAcquisition(handle); }
	);

}

static void stopAcquisition() {

	globals::hDevice->command(
			"stopping acquisition",
			[](auto handle) { return CAEN_DGTZ_SWStopAcquisition(handle); }
	);

	globals::event = nullptr;
	globals::roBuffer = nullptr;

}

INT frontend_init() {
	std::cout << "frontend_init()" << std::endl;

	int status = SUCCESS;

	try {
		// create subtree
		odb::getValueInt32(hDB, 0, "/equipment/" EQUIP_NAME "/Settings/link_num",
				TRUE, defaults::linkNum);

		create_event_rb(test_rbh);
		ss_thread_create(test_thread, 0);

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

	std::cout << "frontend_exit()" << std::endl;

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

INT begin_of_run(INT run_number, char *error) {

	std::cout << "begin_of_run(" << run_number << ", " << error << ")"
			<< std::endl;

	int status = SUCCESS;

	try {
		globals::hDevice = std::make_unique<caen::Handle>(connect());
		configure(*globals::hDevice);
		startAcquisition();

		test_rb_wait_count = 0;
		test_run_number = run_number; // tell thread to start running

	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	} catch (caen::Exception& ex) {
		status = handleCaenException(ex);
	}

	return status;

}

INT end_of_run(INT run_number, char *error) {

	std::cout << "end_of_run(" << run_number << ", " << error << ")"
			<< std::endl;

	int status = SUCCESS;

	try {

		if (globals::hDevice) {
			stopAcquisition();
			globals::hDevice = nullptr;
		}

		test_run_number = 0; // tell thread to stop running

	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	} catch (caen::Exception& ex) {
		status = handleCaenException(ex);
	}

	return status;

}

INT pause_run(INT run_number, char *error) {

	std::cout << "pause_run(" << run_number << ", " << error << ")"
			<< std::endl;

	int status = SUCCESS;

	try {

		stopAcquisition();

		test_run_number = 0; // tell thread to stop running

	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	} catch (caen::Exception& ex) {
		status = handleCaenException(ex);
	}

	return status;

}

INT resume_run(INT run_number, char *error) {

	std::cout << "resume_run(" << run_number << ", " << error << ")"
			<< std::endl;

	int status = SUCCESS;

	try {

		startAcquisition();

		test_run_number = run_number; // tell thread to start running

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

	int result;

	try {
		uint32_t const dataSize = globals::roBuffer->readData();

		uint32_t const numEvents = globals::roBuffer->getNumEvents(dataSize);

		if (!numEvents) {
			return 0;	//	no events
		}

		std::pair<CAEN_DGTZ_EventInfo_t, char*> evt = globals::roBuffer->getEventInfo();
		CAEN_DGTZ_EventInfo_t const& eventInfo = evt.first;

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
