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
#include <util/TriggerInfoRawData.hxx>
#include <util/TDcOffsetRawData.hxx>
#include <util/TWaveFormRawData.hxx>
#include <util/FrontEndUtils.hxx>
#include <caen/handle.hxx>
#include <caen/error-holder.hxx>
#include <caen/exception.hxx>
#include <caen/v1724.hxx>
#include <caen/device.hxx>
#include <caen/fe-commons.hxx>

#include "fe-v1724.hxx"
#include "defaults.hxx"

using namespace fe::v1724;

namespace glob {

static std::vector<uint16_t> dcOffsets;
static uint8_t triggerChannel;
static uint16_t triggerThreshold;
static bool triggerRaisingPolarity;
static uint32_t preTriggerLength;

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

extern HNDLE hDB;

int readEvent(char *pevent, int off);

/*-- Equipment list ------------------------------------------------*/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

EQUIPMENT equipment[] = { { EQUIP_NAME "%02d", { EVID, (1 << EVID), /* event ID, trigger mask */
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

#ifndef NEED_NO_EXTERN_C
}
#endif

static void configure(caen::Handle& hDevice) {

	auto const hSet = util::FrontEndUtils::settingsKey(equipment[0].name);

	fe::commons::configure(hDevice, hSet);

	auto& boardInfo = fe::commons::glob::boardInfo;
	hDevice.hCommand("getting digitizer info",
			[&boardInfo](int handle) {return CAEN_DGTZ_GetInfo(handle, &boardInfo);});

	if (boardInfo.Model != CAEN_DGTZ_V1724) {
		throw caen::Exception(CAEN_DGTZ_GenericError,
				"The device is not CAEN V1724");
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

	auto const recordLength = odb::getValueUInt32(hDB, hSet,
			settings::waveformLength, defaults::recordLength, true);
	if (recordLength > caen::v1724::MAX_RECORD_LENGTH) {
		throw midas::Exception(FE_ERR_ODB,
				std::string("Value of waveform_length parameter exceeds ")
						+ std::to_string(caen::v1724::MAX_RECORD_LENGTH));
	}

	auto const enabledChannels = odb::getValueBoolV(hDB, hSet,
			settings::enabledChannels, boardInfo.Channels,
			fe::commons::defaults::channel::enabled, true);

	glob::dcOffsets = odb::getValueUInt16V(hDB, hSet, settings::channelDcOffset,
			boardInfo.Channels, defaults::channel::dcOffset, true);

	auto const triggerChannel = glob::triggerChannel = odb::getValueUInt8(hDB,
			hSet, settings::triggerChannel, defaults::triggerChannel, true);
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

	auto const triggerThreshold = glob::triggerThreshold = odb::getValueUInt16(
			hDB, hSet, settings::triggerThreshold, defaults::triggerThreshold,
			true);
	hDevice.hCommand("setting channel trigger threshold",
			[triggerChannel, triggerThreshold](int handle) {return CAEN_DGTZ_SetChannelTriggerThreshold(handle, triggerChannel, triggerThreshold);});

	auto const triggerRaisingPolarity = glob::triggerRaisingPolarity =
			odb::getValueBool(hDB, hSet, settings::triggerRaisingPolarity,
					defaults::triggerRaisingPolarity, true);
	hDevice.hCommand("setting trigger polarity",
			[triggerChannel, triggerRaisingPolarity](int handle) {return CAEN_DGTZ_SetTriggerPolarity(handle, triggerChannel, triggerRaisingPolarity ? CAEN_DGTZ_TriggerOnRisingEdge : CAEN_DGTZ_TriggerOnFallingEdge);});

	hDevice.hCommand("setting max num events",
			[](int handle) {return CAEN_DGTZ_SetMaxNumEventsBLT(handle, MAX_NUM_OF_EVENTS);});

	hDevice.hCommand("setting acquisition mode",
			[](int handle) {return CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);});

	auto const regData = hDevice.readRegister(caen::v1724::REG_CHANNEL_CONFIG);
	if (regData & caen::v1724::REG_BIT_TRIGGER_OVERLAP) {
		// disable trigger overlap
		hDevice.writeRegister(caen::v1724::REG_CHANNEL_CONFIG,
				regData & ~caen::v1724::REG_BIT_TRIGGER_OVERLAP);
	}

	auto const preTriggerLength = glob::preTriggerLength = odb::getValueUInt32(
			hDB, hSet, settings::preTriggerLength, defaults::preTriggerLength,
			true);
	if (preTriggerLength > recordLength) {
		throw midas::Exception(FE_ERR_ODB,
				std::string("Invalid value of pre_trigger_length parameter: ")
						+ std::to_string(preTriggerLength)
						+ " is greater than wave form length ("
						+ std::to_string(recordLength) + " samples)");
	}

	hDevice.writeRegister(caen::v1724::REG_POST_TRIGGER,
			(recordLength - preTriggerLength) / 4);

}

namespace fe {

namespace commons {

void configureDevice(caen::Handle& hDevice) {

	::configure(hDevice);

}

}

}

static std::size_t calculateEventSize(CAEN_DGTZ_EventInfo_t const& eventInfo,
		CAEN_DGTZ_UINT16_EVENT_t const& event) {

	// count number of active channels
	unsigned numOfActiveChannels = 0, recordLength = 0;
	for (unsigned i = 0; i < fe::commons::glob::boardInfo.Channels; i++) {
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
		fe::commons::stopAcquisition(*fe::commons::glob::device);
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
		info->frontendIndex = util::FrontEndUtils::frontendIndex<
				decltype(info->frontendIndex)>();
		info->preTriggerLength = glob::preTriggerLength;
		info->triggerMode = 0;
		bk_close(pevent, pdata + sizeof(*info));
	}

	{
		// store trigger information
		uint8_t* pdata;
		bk_create(pevent, util::TriggerInfoRawData::bankName(), TID_WORD,
				(void**) &pdata);
		util::TriggerBank* bank = (util::TriggerBank*) pdata;
		bank->triggerChannel = glob::triggerChannel;
		bank->triggerThreshold = glob::triggerThreshold;
		bank->triggerRising = glob::triggerRaisingPolarity ? 1 : 0;
		bank->reserved = 0;
		bk_close(pevent, pdata + sizeof(*bank));
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
	for (std::size_t i = 0; i < fe::commons::glob::boardInfo.Channels; i++) {
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

	std::lock_guard<decltype(fe::commons::glob::mDevice)> lock(
			fe::commons::glob::mDevice);
	int result;

	if (fe::commons::glob::acquisitionIsOn.load(std::memory_order_relaxed)) {
		result = util::FrontEndUtils::commandR([pevent] {

			CAEN_DGTZ_EventInfo_t eventInfo;
			auto const event = fe::commons::glob::device->nextEvent(eventInfo);
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
