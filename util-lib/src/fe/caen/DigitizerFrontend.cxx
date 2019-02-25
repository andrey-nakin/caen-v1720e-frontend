#include <cstring>
#include <fe/caen/DigitizerFrontend.hxx>
#include <util/FrontEndUtils.hxx>
#include <util/types.hxx>
#include <util/TriggerInfoRawData.hxx>
#include <util/TDcOffsetRawData.hxx>
#include <util/TWaveFormRawData.hxx>
#include <midas/odb.hxx>

namespace fe {

namespace caen {

DigitizerFrontend::DigitizerFrontend() :
		acquisitionIsOn(false) {
}

void DigitizerFrontend::doInitSynchronized() {

	odb::getValueInt32(hDB, 0,
			util::FrontEndUtils::settingsKeyName(equipment[0].name, "link_num"),
			defaults::linkNum, true);

	auto hDevice = connect();
	configure(hDevice);

}

void DigitizerFrontend::doExitSynchronized() {

	if (device) {
		stopAcquisition(*device);
		device = nullptr;
	}

}

void DigitizerFrontend::doBeginOfRunSynchronized(INT /* run_number */,
		char* /* error */) {

	::caen::Handle handle = connect();
	configure(handle);

	device.reset(new ::caen::Device(std::move(handle)));
	startAcquisition(*device);

}

void DigitizerFrontend::doEndOfRunSynchronized(INT /* run_number */,
		char* /* error */) {

	if (device) {
		try {
			stopAcquisition(*device);
		} catch (::caen::Exception& e) {
			util::FrontEndUtils::handleCaenException(e);
		}

		device = nullptr;
	}

}

void DigitizerFrontend::doPauseRunSynchronized(INT /* run_number */,
		char* /* error */) {

	stopAcquisition(*device);

}

void DigitizerFrontend::doResumeRunSynchronized(INT /* run_number */,
		char* /* error */) {

	startAcquisition(*device);

}

int DigitizerFrontend::doPollSynchronized() {

	return acquisitionIsOn.load(std::memory_order_relaxed)
			&& device->hasNextEvent() ? TRUE : FALSE;

}

int DigitizerFrontend::doReadEventSynchronized(char* const pevent,
		int /* off */) {

	if (acquisitionIsOn.load(std::memory_order_relaxed)) {
		CAEN_DGTZ_EventInfo_t eventInfo;
		auto const event = device->nextEvent(eventInfo);
		if (event) {
			return parseEvent(pevent, eventInfo, *event);
		}
	}

	return 0;

}

::caen::Handle DigitizerFrontend::connect() {

	// save reference to settings tree
	auto const hSet = util::FrontEndUtils::settingsKey(equipment[0].name);

	auto const linkNum = odb::getValueInt32(hDB, hSet, settings::linkNum,
			defaults::linkNum, true);
	auto const conetNode = odb::getValueInt32(hDB, hSet, settings::conetNode,
			defaults::conetNode, true);
	auto const vmeBaseAddr = odb::getValueUInt32(hDB, hSet,
			settings::vmeBaseAddr, defaults::vmeBaseAddr, true);

	return ::caen::Handle(linkNum, conetNode, vmeBaseAddr);

}

void DigitizerFrontend::checkBoardInfo(
		CAEN_DGTZ_BoardInfo_t const& /* boardInfo */) {

	// null implementation

}

void DigitizerFrontend::configure(::caen::Handle& hDevice) {

	hDevice.hCommand("getting digitizer info", [this](int handle) {
		return CAEN_DGTZ_GetInfo(handle, &boardInfo);
	});
	checkBoardInfo(boardInfo);

	auto const hSet = util::FrontEndUtils::settingsKey(equipment[0].name);

	hDevice.hCommand("resetting digitizer", CAEN_DGTZ_Reset);

	hDevice.hCommand("setting run sync mode",
			[](int handle) {
				return CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled);
			});

	auto const recordLength = odb::getValueUInt32(hDB, hSet,
			settings::waveformLength, defaults::recordLength, true);
	if (recordLength > getMaxRecordLength()) {
		throw midas::Exception(FE_ERR_ODB,
				std::string("Value of waveform_length parameter exceeds ")
						+ std::to_string(getMaxRecordLength()));
	}

	auto const enabledChannels = odb::getValueBoolV(hDB, hSet,
			settings::enabledChannels, boardInfo.Channels,
			defaults::channel::enabled, true);

	dcOffsets = odb::getValueUInt16V(hDB, hSet, settings::channelDcOffset,
			boardInfo.Channels, defaults::channel::dcOffset, true);

	triggerChannel = odb::getValueInt8(hDB, hSet, settings::triggerChannel,
			defaults::triggerChannel, true);
	if (triggerChannel >= static_cast<int8_t>(boardInfo.Channels)) {
		throw midas::Exception(FE_ERR_ODB,
				std::string("Invalid trigger channel: ")
						+ std::to_string(triggerChannel));
	}
	hDevice.hCommand("setting channel self trigger", [this](int handle) {
		return CAEN_DGTZ_SetChannelSelfTrigger(
				handle,
				CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT,
				triggerChannel >= 0 ? (1 << triggerChannel) : 0
		);
	});

	uint32_t channelMask = triggerChannel >= 0 ? 0x0001 << triggerChannel : 0;
	for (std::size_t i = 0; i != enabledChannels.size(); i++) {
		if (enabledChannels[i]) {
			channelMask |= 0x0001 << i;
		}

		hDevice.hCommand("setting record length",
				[recordLength, i](int handle) {return CAEN_DGTZ_SetRecordLength(handle, recordLength, i);});

		auto const& dcOffset = dcOffsets[i];
		hDevice.hCommand("setting channel DC offset",
				[dcOffset, i](int handle) {return CAEN_DGTZ_SetChannelDCOffset(handle, i, dcOffset);});
	}

	hDevice.hCommand("setting channel enable mask", [channelMask](int handle) {
		return CAEN_DGTZ_SetChannelEnableMask(handle, channelMask);
	});

	triggerThreshold = odb::getValueUInt16(hDB, hSet,
			settings::triggerThreshold, defaults::triggerThreshold, true);
	if (triggerChannel >= 0) {
		hDevice.hCommand("setting channel trigger threshold",
				[this](int handle) {
					return CAEN_DGTZ_SetChannelTriggerThreshold(handle, triggerChannel, triggerThreshold);
				});
	}

	triggerRaisingPolarity = odb::getValueBool(hDB, hSet,
			settings::triggerRaisingPolarity, defaults::triggerRaisingPolarity,
			true);
	if (triggerChannel > 0) {
		hDevice.hCommand("setting trigger polarity",
				[this](int handle) {
					return CAEN_DGTZ_SetTriggerPolarity(handle, triggerChannel, triggerRaisingPolarity ? CAEN_DGTZ_TriggerOnRisingEdge : CAEN_DGTZ_TriggerOnFallingEdge);
				});
	}

	// external trigger
	{
		auto const v = odb::getValueBool(hDB, hSet, settings::extTrigger,
				defaults::extTrigger, true);
		hDevice.hCommand("setting ext trigger input mode",
				[v](int handle) {
					return CAEN_DGTZ_SetExtTriggerInputMode(
							handle,
							v ? CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT : CAEN_DGTZ_TRGMODE_DISABLED
					);
				});
	}

	// post-trigger configuration
	{
		auto const recordLength = odb::getValueUInt32(hDB, hSet,
				settings::waveformLength, defaults::recordLength, true);
		preTriggerLength = odb::getValueUInt32(hDB, hSet,
				settings::preTriggerLength, defaults::preTriggerLength, true);
		if (preTriggerLength > recordLength) {
			throw midas::Exception(FE_ERR_ODB,
					std::string(
							"Invalid value of pre_trigger_length parameter: ")
							+ std::to_string(preTriggerLength)
							+ " is greater than wave form length ("
							+ std::to_string(recordLength) + " samples)");
		}

		hDevice.writeRegister(::caen::reg::POST_TRIG_ADD,
				(recordLength - preTriggerLength) / 4);
	}

	// disable trigger overlap
	hDevice.setBit(::caen::reg::BROAD_CH_CTRL_ADD,
			::caen::regbit::config::TRIGGER_OVERLAP, false);

	// set front panel IO level
	{
		static std::vector<std::string> const IO_LEVELS = { iolevel::nim,
				iolevel::ttl };
		auto const v = odb::getValueString(hDB, hSet,
				settings::frontPanelIOLevel, IO_LEVELS);
		hDevice.setBit(::caen::reg::FRONT_PANEL_IO_CTRL_ADD,
				::caen::regbit::fpioctl::IO_LEVEL, v == iolevel::ttl);
	}

	hDevice.hCommand("setting max num events", [](int handle) {
		return CAEN_DGTZ_SetMaxNumEventsBLT(handle, MAX_NUM_OF_EVENTS);
	});

	hDevice.hCommand("setting acquisition mode", [](int handle) {
		return CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
	});

}

void DigitizerFrontend::startAcquisition(::caen::Device& device) {

	device.startAcquisition();
	acquisitionIsOn.store(true);

}

void DigitizerFrontend::stopAcquisition(::caen::Device& device) {

	acquisitionIsOn.store(false);
	device.stopAcquisition();

}

void DigitizerFrontend::storeTriggerBank(char* const pevent) {

	if (triggerChannel < 0) {
		return;
	}

	uint8_t* pdata;
	bk_create(pevent, util::TriggerInfoRawData::bankName(), TID_WORD,
			(void**) &pdata);
	util::TriggerBank* bank = (util::TriggerBank*) pdata;
	bank->triggerChannel = triggerChannel;
	bank->triggerThreshold = triggerThreshold;
	bank->triggerRising = triggerRaisingPolarity ? 1 : 0;
	bank->reserved = 0;
	bk_close(pevent, pdata + sizeof(*bank));

}

void DigitizerFrontend::storeDcOffsetBank(char* const pevent) {

	uint16_t* pdata;
	bk_create(pevent, util::TDcOffsetRawData::BANK_NAME, TID_WORD,
			(void**) &pdata);
	for (auto const& dcOffset : dcOffsets) {
		*pdata++ = dcOffset;
	}
	bk_close(pevent, pdata);

}

void DigitizerFrontend::storeWaveformBanks(char* const pevent,
		CAEN_DGTZ_EventInfo_t const& eventInfo,
		CAEN_DGTZ_UINT16_EVENT_t const& event) {

	for (std::size_t i = 0; i < boardInfo.Channels; i++) {
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

}

int DigitizerFrontend::parseEvent(char* const pevent,
		CAEN_DGTZ_EventInfo_t const& eventInfo,
		CAEN_DGTZ_UINT16_EVENT_t const& event) {

	// check event size
	auto const eventSize = calculateEventSize(eventInfo, event);
	if (eventSize > static_cast<DWORD>(max_event_size)) {
		// event size exceeds the limit
		stopAcquisition(*device);
		cm_msg(MERROR, frontend_name,
				"Event size %" PRIu32 " larger than maximum size %" PRIi32,
				static_cast<DWORD>(eventSize), max_event_size);
		return 0;
	}

	bk_init32(pevent);

	{
		// store general information
		uint8_t* pdata;
		bk_create(pevent, infoBankName(), TID_DWORD, (void**) &pdata);
		util::InfoBank* info = (util::InfoBank*) pdata;
		info->boardId = eventInfo.BoardId;
		info->channelMask = eventInfo.ChannelMask;
		info->eventCounter = eventInfo.EventCounter;
		info->timeStampLo = eventInfo.TriggerTimeTag;
		info->timeStampHi = eventInfo.Pattern;
		info->frontendIndex = util::FrontEndUtils::frontendIndex<
				decltype(info->frontendIndex)>();
		info->preTriggerLength = preTriggerLength;
		info->triggerMode = 0;
		bk_close(pevent, pdata + sizeof(*info));
	}

	storeTriggerBank(pevent);
	storeDcOffsetBank(pevent);
	storeWaveformBanks(pevent, eventInfo, event);

	return bk_size(pevent);

}

}

}
