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
#include <midas.h>
#include <msystem.h>

#include <midas/odb.hxx>
#include <frontend/types.hxx>
#include <caen/handle.hxx>
#include <caen/error-holder.hxx>
#include <caen/readout-buffer.hxx>
#include <caen/event.hxx>
#include <caen/exception.hxx>
#include <caen/v1720.hxx>

#include "defaults.hxx"

#define EQUIP_NAME "v1720"
constexpr uint32_t MAX_NUM_OF_EVENTS = 1;
constexpr int32_t FIRST_EVENT = 0;
constexpr int EVID = 1;

namespace glob {

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
static std::atomic_bool acquisitionIsOn(false);
static std::mutex readingMutex;

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
INT max_event_size = 4194304 / 8;
INT max_event_size_frag = 4194304 / 8;

/* buffer size to hold events */
INT event_buffer_size = 4194304;

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

int test_rb_wait_sleep = 1;
int test_rb_wait_count = 0;
int test_rbh = 0;

static int workingThread(void * /*param */) {
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

template<typename T>
static std::string toString(T const v, std::size_t const len) {

	auto s = std::to_string(v);
	while (s.size() < len) {
		s = "0" + s;
	}
	return s;

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

	auto const linkNum = odb::getValueInt32(hDB, hSet, "link_num",
			defaults::linkNum, true);
	auto const conetNode = odb::getValueInt32(hDB, hSet, "conet_node",
			defaults::conetNode, true);
	auto const vmeBaseAddr = odb::getValueUInt32(hDB, hSet, "vme_base_addr",
			defaults::vmeBaseAddr, true);

	return caen::Handle(linkNum, conetNode, vmeBaseAddr);

}

static void configure(caen::Handle& hDevice) {

	auto const hSet = getSettingsKey();

	auto& boardInfo = glob::boardInfo;
	hDevice.hCommand("getting digitizer info",
			[&boardInfo](int handle) {return CAEN_DGTZ_GetInfo(handle, &boardInfo);});

	if (boardInfo.Model != CAEN_DGTZ_V1720) {
		throw caen::Exception(CAEN_DGTZ_GenericError,
				"The device is not CAEN V1720");
	}

	int majorNumber;
	sscanf(boardInfo.AMC_FirmwareRel, "%d", &majorNumber);
	if (majorNumber >= 128) {
		throw caen::Exception(CAEN_DGTZ_GenericError,
				"This digitizer has a DPP firmware");
	}

	hDevice.hCommand("resetting digitizer", CAEN_DGTZ_Reset);

	hDevice.hCommand("setting IO level",
			[](int handle) {return CAEN_DGTZ_SetIOLevel(handle, CAEN_DGTZ_IOLevel_NIM);});

	hDevice.hCommand("setting external trigger input mode",
			[](int handle) {return CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);});

	hDevice.hCommand("setting run sync mode",
			[](int handle) {return CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled);});

	auto const recordLength = odb::getValueUInt32(hDB, hSet, "waveform_length",
			defaults::recordLength, true);
	glob::recordLength = recordLength;

	glob::enabledChannels = odb::getValueBoolV(hDB, hSet, "channel_enabled",
			boardInfo.Channels, defaults::channel::enabled, true);

	glob::dcOffsets = odb::getValueUInt16V(hDB, hSet, "channel_dc_offset",
			boardInfo.Channels, defaults::channel::dcOffset, true);

	uint32_t channelMask = 0x0000;
	for (std::size_t i = 0; i != glob::enabledChannels.size(); i++) {
		if (glob::enabledChannels[i]) {
			channelMask |= 0x0001 << i;
		}

		hDevice.hCommand("setting record length",
				[recordLength, i](int handle) {return CAEN_DGTZ_SetRecordLength(handle, recordLength, i);});

		auto const& dcOffset = glob::dcOffsets[i];
		hDevice.hCommand("setting channel DC offset",
				[dcOffset, i](int handle) {return CAEN_DGTZ_SetChannelDCOffset(handle, i, dcOffset);});
	}

	hDevice.hCommand("setting channel enable mask",
			[channelMask](int handle) {return CAEN_DGTZ_SetChannelEnableMask(handle, channelMask);});

	auto const triggerMode = odb::getValueString(hDB, hSet, "trigger_mode",
			defaults::triggerMode, true);

	auto const triggerChannel = odb::getValueUInt8(hDB, hSet, "trigger_channel",
			defaults::triggerChannel, true);
	hDevice.hCommand("setting channel self trigger",
			[triggerChannel](int handle) {return CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, (1 << triggerChannel));});

	auto const triggerThreshold = odb::getValueUInt16(hDB, hSet,
			"trigger_threshold", defaults::triggerThreshold, true);
	hDevice.hCommand("setting channel trigger threshold",
			[triggerChannel, triggerThreshold](int handle) {return CAEN_DGTZ_SetChannelTriggerThreshold(handle, triggerChannel, triggerThreshold);});

	auto const triggerRaisingPolarity = odb::getValueBool(hDB, hSet,
			"trigger_raising_polarity", defaults::triggerRaisingPolarity, true);
	hDevice.hCommand("setting trigger polarity",
			[triggerChannel, triggerRaisingPolarity](int handle) {return CAEN_DGTZ_SetTriggerPolarity(handle, triggerChannel, triggerRaisingPolarity ? CAEN_DGTZ_TriggerOnRisingEdge : CAEN_DGTZ_TriggerOnFallingEdge);});

	hDevice.hCommand("setting max num events",
			[](int handle) {return CAEN_DGTZ_SetMaxNumEventsBLT(handle, MAX_NUM_OF_EVENTS);});

	hDevice.hCommand("setting acquisition mode",
			[](int handle) {return CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);});

	auto const regData = hDevice.readRegister(caen::v1720::REG_CHANNEL_CONFIG);
	if (regData & caen::v1720::REG_BIT_TRIGGER_OVERLAP) {
		// disable trigger overlap
		hDevice.writeRegister(caen::v1720::REG_CHANNEL_CONFIG,
				regData & ~caen::v1720::REG_BIT_TRIGGER_OVERLAP);
	}

	glob::preTriggerLength = odb::getValueUInt32(hDB, hSet,
			"pre_trigger_length", defaults::preTriggerLength, true);

	hDevice.writeRegister(caen::v1720::REG_POST_TRIGGER,
			(glob::recordLength - glob::preTriggerLength) / 4);

}

static void startAcquisition() {

	glob::roBuffer = std::unique_ptr < caen::ReadoutBuffer
			> (new caen::ReadoutBuffer(*glob::hDevice));

	glob::event = std::unique_ptr < caen::Event
			> (new caen::Event(*glob::hDevice));

	glob::hDevice->hCommand("starting acquisition",
			CAEN_DGTZ_SWStartAcquisition);

	glob::acquisitionIsOn.store(true);

}

static void stopAcquisition() {

	glob::acquisitionIsOn.store(false);

	std::lock_guard < std::mutex > lock(glob::readingMutex);

	glob::hDevice->hCommand("stopping acquisition",
			CAEN_DGTZ_SWStopAcquisition);

	glob::event = nullptr;
	glob::roBuffer = nullptr;

}

INT frontend_init() {
	int status = SUCCESS;

	try {
		// create subtree
		odb::getValueInt32(hDB, 0,
				"/equipment/" EQUIP_NAME "/Settings/link_num",
				defaults::linkNum, true);

		create_event_rb(test_rbh);
		glob::readoutThread = ss_thread_create(workingThread, 0);

		caen::Handle hDevice = connect();
		configure(hDevice);

	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	} catch (caen::Exception& ex) {
		handleCaenException(ex);
	}

	return status;
}

INT frontend_exit() {

	int status = SUCCESS;

	try {

		if (glob::hDevice) {
			stopAcquisition();
			glob::hDevice = nullptr;
		}

		ss_thread_kill(glob::readoutThread);

	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	} catch (caen::Exception& ex) {
		status = handleCaenException(ex);
	}

	return status;

}

INT begin_of_run(INT /* run_number */, char * /* error */) {

	int status = SUCCESS;

	try {
		glob::hDevice = std::unique_ptr < caen::Handle
				> (new caen::Handle(connect()));
		configure(*glob::hDevice);

		startAcquisition();

		test_rb_wait_count = 0;

	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	} catch (caen::Exception& ex) {
		status = handleCaenException(ex);
	}

	return status;

}

INT end_of_run(INT /* run_number */, char * /* error */) {

	int status = SUCCESS;

	try {

		if (glob::hDevice) {
			stopAcquisition();
			glob::hDevice = nullptr;
		}

	} catch (midas::Exception& ex) {
		status = ex.getStatus();
	} catch (caen::Exception& ex) {
		status = handleCaenException(ex);
	}

	return status;

}

INT pause_run(INT /* run_number */, char * /* error */) {

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

INT resume_run(INT /* run_number */, char * /* error */) {

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

static int parseEvent(char * const pevent, uint32_t const dataSize,
		int32_t const numEvent) {

	std::pair<CAEN_DGTZ_EventInfo_t, char*> evt = glob::roBuffer->getEventInfo(
			dataSize, numEvent);
	CAEN_DGTZ_EventInfo_t const& eventInfo = evt.first;

	if (eventInfo.EventCounter > glob::eventCounter + 1) {
		cm_msg(MERROR, frontend_name, "%u event(s) has been lost",
				eventInfo.EventCounter - glob::eventCounter - 1);
	}
	glob::eventCounter = eventInfo.EventCounter;

	glob::event->decode(evt.second);

	bk_init32(pevent);

	{
		// store general information
		uint8_t* pdata;
		bk_create(pevent, "INFO", TID_DWORD, (void**) &pdata);
		fe::InfoBank* info = (fe::InfoBank*) pdata;
		info->dataType = fe::DataType::WaveForm16bitVer1;
		info->deviceType = fe::DeviceType::CaenV1720E;
		info->boardId = eventInfo.BoardId;
		info->channelMask = eventInfo.ChannelMask;
		info->eventCounter = eventInfo.EventCounter;
		info->timeStamp = (uint64_t) eventInfo.Pattern << 32
				| eventInfo.TriggerTimeTag;
		info->recordLength = glob::recordLength;
		info->preTriggerLength = glob::preTriggerLength;
		bk_close(pevent, pdata + sizeof(*info));
	}

	{
		// store channel DC offset
		uint16_t* pdata;
		bk_create(pevent, "CHDC", TID_WORD, (void**) &pdata);
		for (auto const& dcOffset : glob::dcOffsets) {
			*pdata++ = dcOffset;
		}
		bk_close(pevent, pdata);
	}

	// store wave forms
	for (std::size_t i = 0; i < glob::boardInfo.Channels; i++) {
		if (eventInfo.ChannelMask & (0x0001 << i)) {
			auto const numOfSamples = glob::event->evt()->ChSize[i];
			if (numOfSamples > 0) {
				uint16_t const * const samples =
						glob::event->evt()->DataChannel[i];
				auto const dataSize = numOfSamples * sizeof(samples[0]);

				std::string const name = "WF" + toString(i, 2);
				uint8_t* pdata;
				bk_create(pevent, name.c_str(), TID_WORD, (void**) &pdata);
				std::memcpy(pdata, samples, dataSize);
				bk_close(pevent, pdata + dataSize);
			}
		}
	}

	return bk_size(pevent);

}

int readEvent(char * const pevent, const int /* off */) {

	std::lock_guard < std::mutex > lock(glob::readingMutex);
	int result;

	if (glob::acquisitionIsOn.load(std::memory_order_relaxed)) {
		try {
			auto const dataSize = glob::roBuffer->readData();

			auto const numEvents = glob::roBuffer->getNumEvents(dataSize);

			result =
					numEvents > 0 ?
							parseEvent(pevent, dataSize, FIRST_EVENT) : 0;

		} catch (midas::Exception& ex) {
			result = 0;
		} catch (caen::Exception& ex) {
			handleCaenException(ex);
			result = 0;
		}
	} else {
		result = 0;
	}

	return result;

}
