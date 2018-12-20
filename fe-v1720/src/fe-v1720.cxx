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
#include <util/types.hxx>
#include <util/V1720InfoRawData.hxx>
#include <util/TDcOffsetRawData.hxx>
#include <util/TWaveFormRawData.hxx>
#include <util/FrontEndUtils.hxx>
#include <caen/handle.hxx>
#include <caen/error-holder.hxx>
#include <caen/exception.hxx>
#include <caen/v1720.hxx>
#include <caen/device.hxx>

#include "defaults.hxx"

#define EQUIP_NAME "v1720"
constexpr uint32_t MAX_NUM_OF_EVENTS = 1;
constexpr int32_t FIRST_EVENT = 0;
constexpr int EVID = 1;
constexpr uint32_t MAX_RECORD_LENGTH = 1024 * 1024;

namespace glob {

static CAEN_DGTZ_BoardInfo_t boardInfo;
static std::vector<uint16_t> dcOffsets;
static std::unique_ptr<caen::Device> device;
static uint32_t eventCounter;
static midas_thread_t readoutThread;
static std::atomic_bool acquisitionIsOn(false);
static std::mutex readingMutex;
static int const rbWaitSleep = 1;
static int rbWaitCount = 0;
static int rbh = 0;

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

EQUIPMENT equipment[] = { { EQUIP_NAME "%02d", { EVID, (1 << EVID), /* event ID, trigger mask */
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

static int workingThread(void * /*param */) {
	EQUIPMENT* eq = &equipment[0];

	/* indicate activity to framework */
	signal_readout_thread_active(glob::rbh, 1);

	while (!stop_all_threads) {
		if (!glob::acquisitionIsOn.load(std::memory_order_relaxed)) {
			// no run, wait
			ss_sleep(10);
			continue;
		}

		/* obtain buffer space */
		void *p;
		auto const status = rb_get_wp(get_event_rbh(glob::rbh), &p, 0);
		if (stop_all_threads) {
			break;
		}
		if (status == DB_TIMEOUT) {
			glob::rbWaitCount++;
			cm_msg(MINFO, frontend_name, "No free ring buffers, waiting");
			if (glob::rbWaitSleep) {
				ss_sleep(glob::rbWaitSleep);
			}
			continue;
		}
		if (status != DB_SUCCESS) {
			cm_msg(MERROR, frontend_name, "Cannot get ring buffer, status %d",
					status);
			break;
		}

		/* check for new event */

		if (stop_all_threads) {
			break;
		}

		/* call user readout routine */
		auto pHeader = reinterpret_cast<EVENT_HEADER*>(p);
		auto const dataSize = eq->readout(
				reinterpret_cast<char*>(p) + sizeof(*pHeader), 0);

		if (dataSize > 0) {
			/* an event arrived, check its size */
			auto const eventSize = dataSize + sizeof(*pHeader);
			if (eventSize > static_cast<DWORD>(max_event_size)) {
				cm_msg(MERROR, frontend_name,
						"Event size %" PRIu32 " larger than maximum size %" PRIi32,
						static_cast<DWORD>(eventSize), max_event_size);
				break;
			}

			/* compose MIDAS event header */
			pHeader->data_size = dataSize;
			pHeader->event_id = eq->info.event_id;
			pHeader->trigger_mask = eq->info.trigger_mask;
			pHeader->time_stamp = ss_time();
			pHeader->serial_number = eq->serial_number++;

			/* put event into ring buffer */
			rb_increment_wp(get_event_rbh(glob::rbh), eventSize);
		} else {
			/* no events, wait a bit */
			if (eq->info.period) {
				ss_sleep(eq->info.period);
			}
		}
	}

	signal_readout_thread_active(glob::rbh, 0);

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

static caen::Handle connect() {

	// save reference to settings tree
	auto const hSet = util::FrontEndUtils::settingsKey(equipment[0].name);

	auto const linkNum = odb::getValueInt32(hDB, hSet, "link_num",
			defaults::linkNum, true);
	auto const conetNode = odb::getValueInt32(hDB, hSet, "conet_node",
			defaults::conetNode, true);
	auto const vmeBaseAddr = odb::getValueUInt32(hDB, hSet, "vme_base_addr",
			defaults::vmeBaseAddr, true);

	return caen::Handle(linkNum, conetNode, vmeBaseAddr);

}

static void configure(caen::Handle& hDevice) {

	auto const hSet = util::FrontEndUtils::settingsKey(equipment[0].name);

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
	if (recordLength > MAX_RECORD_LENGTH) {
		throw midas::Exception(FE_ERR_ODB,
				std::string("Value of waveform_length parameter exceeds ")
						+ std::to_string(MAX_RECORD_LENGTH));
	}

	auto const enabledChannels = odb::getValueBoolV(hDB, hSet,
			"channel_enabled", boardInfo.Channels, defaults::channel::enabled,
			true);

	glob::dcOffsets = odb::getValueUInt16V(hDB, hSet, "channel_dc_offset",
			boardInfo.Channels, defaults::channel::dcOffset, true);

	auto const triggerChannel = odb::getValueUInt8(hDB, hSet, "trigger_channel",
			defaults::triggerChannel, true);
	hDevice.hCommand("setting channel self trigger",
			[triggerChannel](int handle) {return CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, (1 << triggerChannel));});
	if (triggerChannel >= boardInfo.Channels) {
		throw midas::Exception(FE_ERR_ODB,
				std::string("Invalid trigger channel: ")
						+ std::to_string(triggerChannel));
	}

	uint32_t channelMask = 0x0001 << triggerChannel;
	for (std::size_t i = 0; i != enabledChannels.size(); i++) {
		if (enabledChannels[i]) {
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

	auto const preTriggerLength = odb::getValueUInt32(hDB, hSet,
			"pre_trigger_length", defaults::preTriggerLength, true);
	if (preTriggerLength > recordLength) {
		throw midas::Exception(FE_ERR_ODB,
				std::string("Invalid value of pre_trigger_length parameter: ")
						+ std::to_string(preTriggerLength)
						+ " is greater than wave form length ("
						+ std::to_string(recordLength) + " samples)");
	}

	hDevice.writeRegister(caen::v1720::REG_POST_TRIGGER,
			(recordLength - preTriggerLength) / 4);

}

static void startAcquisition(caen::Device& device) {

	device.startAcquisition();
	glob::acquisitionIsOn.store(true);

}

static void stopAcquisition(caen::Device& device) {

	glob::acquisitionIsOn.store(false);

	std::lock_guard < std::mutex > lock(glob::readingMutex);

	device.stopAcquisition();

}

INT frontend_init() {

	return util::FrontEndUtils::command([]() {

		odb::getValueInt32(hDB, 0,
				util::FrontEndUtils::settingsKeyName(equipment[0].name,
						"link_num"), defaults::linkNum, true);

		create_event_rb(glob::rbh);
		glob::readoutThread = ss_thread_create(workingThread, 0);

		caen::Handle hDevice = connect();
		configure(hDevice);

	});

}

INT frontend_exit() {

	return util::FrontEndUtils::command([]() {

		std::lock_guard < std::mutex > lock(glob::readingMutex);

		if (glob::device) {
			stopAcquisition(*glob::device);
			glob::device = nullptr;
		}

		ss_thread_kill(glob::readoutThread);

	});

}

INT begin_of_run(INT /* run_number */, char * /* error */) {

	return util::FrontEndUtils::command([]() {

		glob::device = std::unique_ptr < caen::Device
		> (new caen::Device(connect()));
		configure(glob::device->getHandle());

		startAcquisition(*glob::device);

		glob::rbWaitCount = 0;

	});

}

INT end_of_run(INT /* run_number */, char * /* error */) {

	return util::FrontEndUtils::command([]() {

		std::lock_guard < std::mutex > lock(glob::readingMutex);

		if (glob::device) {
			stopAcquisition(*glob::device);
			glob::device = nullptr;
		}

	});

}

INT pause_run(INT /* run_number */, char * /* error */) {

	return util::FrontEndUtils::command([]() {

		std::lock_guard < std::mutex > lock(glob::readingMutex);

		stopAcquisition(*glob::device);

	});

}

INT resume_run(INT /* run_number */, char * /* error */) {

	return util::FrontEndUtils::command([]() {

		std::lock_guard < std::mutex > lock(glob::readingMutex);

		startAcquisition(*glob::device);

	});

}

INT frontend_loop() {

	return SUCCESS;

}

static int parseEvent(char * const pevent, uint32_t const dataSize,
		int32_t const numEvent) {

	std::pair<CAEN_DGTZ_EventInfo_t, char*> evt =
			glob::device->getBuffer().getEventInfo(dataSize, numEvent);
	CAEN_DGTZ_EventInfo_t const& eventInfo = evt.first;

	if (eventInfo.EventCounter > glob::eventCounter + 1) {
		cm_msg(MERROR, frontend_name, "%u event(s) has been lost",
				eventInfo.EventCounter - glob::eventCounter - 1);
	}
	glob::eventCounter = eventInfo.EventCounter;

	glob::device->getEvent().decode(evt.second);

	bk_init32(pevent);

	{
		// store general information
		uint8_t* pdata;
		bk_create(pevent, util::V1720InfoRawData::bankName(), TID_DWORD,
				(void**) &pdata);
		util::InfoBank* info = (util::InfoBank*) pdata;
		info->boardId = eventInfo.BoardId;
		info->channelMask = eventInfo.ChannelMask;
		info->eventCounter = eventInfo.EventCounter;
		info->timeStampLo = eventInfo.TriggerTimeTag;
		info->timeStampHi = eventInfo.Pattern;
		bk_close(pevent, pdata + sizeof(*info));
	}

	{
		// store channel DC offset
		uint16_t* pdata;
		bk_create(pevent, util::TDcOffsetRawData::BANK_NAME, TID_WORD,
				(void**) &pdata);
		for (auto const& dcOffset : glob::dcOffsets) {
			*pdata++ = dcOffset;
		}
		bk_close(pevent, pdata);
	}

	// store wave forms
	for (std::size_t i = 0; i < glob::boardInfo.Channels; i++) {
		if (eventInfo.ChannelMask & (0x0001 << i)) {
			auto const numOfSamples = glob::device->getEvent().evt()->ChSize[i];
			if (numOfSamples > 0) {
				uint16_t const * const samples =
						glob::device->getEvent().evt()->DataChannel[i];
				auto const dataSize = numOfSamples * sizeof(samples[0]);

				uint8_t* pdata;
				bk_create(pevent, util::TWaveFormRawData::bankName(i), TID_WORD,
						(void**) &pdata);
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
		result =
				util::FrontEndUtils::commandR(
						[pevent]() {

							auto const dataSize = glob::device->getBuffer().readData();

							auto const numEvents = glob::device->getBuffer().getNumEvents(dataSize);

							return numEvents > 0 ? parseEvent(pevent, dataSize, FIRST_EVENT) : 0;

						});
	} else {
		result = 0;
	}

	return result;

}
