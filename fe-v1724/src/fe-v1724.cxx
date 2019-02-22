#include <midas.h>
#include <msystem.h>

#include <midas/odb.hxx>
#include <util/types.hxx>
#include <util/V1720InfoRawData.hxx>
#include <util/FrontEndUtils.hxx>
#include <caen/exception.hxx>
#include <caen/v1724.hxx>
#include <caen/device.hxx>
#include <caen/fe-commons.hxx>

#include "fe-v1724.hxx"

using namespace fe::v1724;

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

EQUIPMENT equipment[] = { { EQUIP_NAME "%02d", { fe::commons::EVID, (1
		<< fe::commons::EVID), /* event ID, trigger mask */
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

	auto const hSet = util::FrontEndUtils::settingsKey(equipment[0].name);
	fe::commons::configure(hDevice, hSet, caen::v1724::V1724Details());

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
		info->preTriggerLength = fe::commons::glob::preTriggerLength;
		info->triggerMode = 0;
		bk_close(pevent, pdata + sizeof(*info));
	}

	fe::commons::storeTriggerBank(pevent);
	fe::commons::storeDcOffsetBank(pevent);
	fe::commons::storeWaveformBanks(pevent, eventInfo, event);

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
