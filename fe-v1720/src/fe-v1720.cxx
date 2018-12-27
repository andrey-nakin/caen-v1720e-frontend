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

#include "fe-v1720.hxx"
#include "defaults.hxx"

constexpr uint32_t MAX_NUM_OF_EVENTS = 100;
constexpr int EVID = 1;
constexpr uint32_t MAX_EVENT_SIZE = calculateEventSize(
		caen::v1720::NUM_OF_CHANNELS, caen::v1720::MAX_RECORD_LENGTH);

namespace glob {

static CAEN_DGTZ_BoardInfo_t boardInfo;
static std::vector<uint16_t> dcOffsets;
static std::unique_ptr<caen::Device> device;
static std::recursive_mutex mDevice;
static std::atomic_bool acquisitionIsOn(false);

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
INT max_event_size = MAX_EVENT_SIZE;
INT max_event_size_frag = MAX_EVENT_SIZE;

/* buffer size to hold events */
INT event_buffer_size = 2 * MAX_EVENT_SIZE;

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
//EQ_USER, /* equipment type */
		EQ_POLLED, /* equipment type */
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
	if (recordLength > caen::v1720::MAX_RECORD_LENGTH) {
		throw midas::Exception(FE_ERR_ODB,
				std::string("Value of waveform_length parameter exceeds ")
						+ std::to_string(caen::v1720::MAX_RECORD_LENGTH));
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
	device.stopAcquisition();

}

INT frontend_init() {

	cm_msg(MDEBUG, frontend_name, "frontend_init");

	return util::FrontEndUtils::command([]() {

		odb::getValueInt32(hDB, 0,
				util::FrontEndUtils::settingsKeyName(equipment[0].name,
						"link_num"), defaults::linkNum, true);

		caen::Handle hDevice = connect();
		configure(hDevice);

	});

}

INT frontend_exit() {

	cm_msg(MDEBUG, frontend_name, "frontend_exit");

	return util::FrontEndUtils::command([]() {

		std::lock_guard<decltype(glob::mDevice)> lock(glob::mDevice);

		if (glob::device) {
			stopAcquisition(*glob::device);
			glob::device = nullptr;
		}

	});

}

INT begin_of_run(INT run_number, char * /* error */) {

	cm_msg(MDEBUG, frontend_name, "begin_of_run run_number=%d", run_number);

	return util::FrontEndUtils::command([]() {

		std::lock_guard<decltype(glob::mDevice)> lock(glob::mDevice);

		glob::device.reset(new caen::Device(connect()));
		configure(glob::device->getHandle());

		startAcquisition(*glob::device);

	});

}

INT end_of_run(INT run_number, char * /* error */) {

	cm_msg(MDEBUG, frontend_name, "end_of_run run_number=%d", run_number);

	return util::FrontEndUtils::command([run_number] {

		std::lock_guard<decltype(glob::mDevice)> lock(glob::mDevice);

		if (glob::device) {
			try {
				stopAcquisition(*glob::device);
			} catch (caen::Exception& e) {
				util::FrontEndUtils::handleCaenException(e);
			}

			glob::device = nullptr;
		}

	});

}

INT pause_run(INT run_number, char * /* error */) {

	cm_msg(MDEBUG, frontend_name, "pause_run run_number=%d", run_number);

	return util::FrontEndUtils::command([]() {

		std::lock_guard<decltype(glob::mDevice)> lock(glob::mDevice);

		stopAcquisition(*glob::device);

	});

}

INT resume_run(INT run_number, char * /* error */) {

	cm_msg(MDEBUG, frontend_name, "resume_run run_number=%d", run_number);

	return util::FrontEndUtils::command([]() {

		std::lock_guard<decltype(glob::mDevice)> lock(glob::mDevice);

		startAcquisition(*glob::device);

	});

}

INT frontend_loop() {

	return SUCCESS;

}

static std::size_t calculateEventSize(CAEN_DGTZ_EventInfo_t const& eventInfo,
		CAEN_DGTZ_UINT16_EVENT_t const& event) {

	// count number of active channels
	unsigned numOfActiveChannels = 0, recordLength = 0;
	for (unsigned i = 0; i < glob::boardInfo.Channels; i++) {
		if (eventInfo.ChannelMask & (0x0001 << i)) {
			numOfActiveChannels++;
			recordLength = std::max(recordLength, event.ChSize[i]);
		}
	}

	return calculateEventSize(numOfActiveChannels, recordLength);

}

static int parseEvent(char * const pevent,
		CAEN_DGTZ_EventInfo_t const& eventInfo,
		CAEN_DGTZ_UINT16_EVENT_t const& event) {

	// check event size
	auto const eventSize = calculateEventSize(eventInfo, event);
	if (eventSize > static_cast<DWORD>(max_event_size)) {
		// event size exceeds the limit
		stopAcquisition(*glob::device);
		cm_msg(MERROR, frontend_name,
				"Event size %" PRIu32 " larger than maximum size %" PRIi32,
				static_cast<DWORD>(eventSize), max_event_size);
		return 0;
	}

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
			auto const numOfSamples = event.ChSize[i];
			if (numOfSamples > 0) {
				uint16_t const * const samples = event.DataChannel[i];
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

	std::lock_guard<decltype(glob::mDevice)> lock(glob::mDevice);
	int result;

	if (glob::acquisitionIsOn.load(std::memory_order_relaxed)) {
		result = util::FrontEndUtils::commandR([pevent] {

			CAEN_DGTZ_EventInfo_t eventInfo;
			auto const event = glob::device->nextEvent(eventInfo);
			if (event) {
				return parseEvent(pevent, eventInfo, *event);
			} else {
				return 0;
			}

		});
	} else {
		result = 0;
	}

	return result;

}

INT poll_event(INT /* source */, INT /* count */, BOOL const test) {

	INT result;

	{
		std::lock_guard<decltype(glob::mDevice)> lock(glob::mDevice);

		result = util::FrontEndUtils::commandR([] {
			return glob::acquisitionIsOn.load(std::memory_order_relaxed)
			&& glob::device->hasNextEvent() ? TRUE : FALSE;
		});
	}

	if (!result) {
		ss_sleep(equipment[0].info.period);
	}

	return result && !test;

}

INT interrupt_configure(INT /* cmd */, INT /* source */, PTYPE /* adr */) {

	return SUCCESS;

}
