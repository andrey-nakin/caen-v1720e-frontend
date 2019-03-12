#include <cstring>
#include <sstream>
#include <fe/caen/DigitizerFrontend.hxx>
#include <util/FrontEndUtils.hxx>
#include <util/types.hxx>
#include <util/TriggerInfoRawData.hxx>
#include <util/TDcOffsetRawData.hxx>
#include <util/TWaveFormRawData.hxx>
#include <util/SignalInfoRawData.hxx>
#include <midas/odb.hxx>

namespace fe {

namespace caen {

DigitizerFrontend::DigitizerFrontend() :
		acquisitionIsOn(false) {
}

std::size_t DigitizerFrontend::calculateEventSize(
		CAEN_DGTZ_EventInfo_t const& eventInfo,
		CAEN_DGTZ_UINT16_EVENT_t const& event) const {

	// count number of active channels
	unsigned numOfActiveChannels = 0, recordLength = 0;
	for (unsigned i = 0; i < boardInfo.Channels; i++) {
		if (eventInfo.ChannelMask & (0x0001 << i)) {
			numOfActiveChannels++;
			recordLength = std::max(recordLength, event.ChSize[i]);
		}
	}

	return calculateEventSize(numOfActiveChannels, recordLength);

}

void DigitizerFrontend::doInitSynchronized() {

	odb::getValueInt32(hDB, 0,
			util::FrontEndUtils::settingsKeyName(equipment[0].name, "link_num"),
			defaults::linkNum, true);

	auto hDevice = connect();
	configure(hDevice);

}

void DigitizerFrontend::doExitSynchronized() {

	closeDevice();

}

void DigitizerFrontend::doBeginOfRunSynchronized(INT /* run_number */,
		char* /* error */) {

	// stop acquisition if previous run was not properly stopped
	closeDevice();

	::caen::Handle handle = connect();
	configure(handle);

	::caen::Device tmp(std::move(handle));
	startAcquisition(tmp);

	device.reset(new ::caen::Device(std::move(tmp)));

}

void DigitizerFrontend::doEndOfRunSynchronized(INT /* run_number */,
		char* /* error */) {

	closeDevice();

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

	{
		std::stringstream ss;
		ss << "digitizer model: " << boardInfo.ModelName
				<< ", ROC FPGA Release: " << boardInfo.ROC_FirmwareRel;
		std::string s = ss.str();
		cm_msg(MINFO, frontend_name, s.c_str());
	}

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
	signalFrontLengths = odb::getValueUInt32V(hDB, hSet,
			settings::signalFrontLength, boardInfo.Channels,
			defaults::channel::signalFrontLength, true);
	signalLengths = odb::getValueUInt32V(hDB, hSet, settings::signalLength,
			boardInfo.Channels, defaults::channel::signalLength, true);
	signalRisingPolarities = odb::getValueBoolV(hDB, hSet,
			settings::signalRisingPolarity, boardInfo.Channels,
			defaults::channel::signalRisingPolarity, true);
	signalTriggerChannel = odb::getValueUInt8V(hDB, hSet,
			settings::signalTriggerChannel, boardInfo.Channels,
			defaults::channel::signalTriggerChannel, true);

	triggerChannel = odb::getValueBoolV(hDB, hSet, settings::triggerChannel,
			boardInfo.Channels, defaults::triggerChannel, true);
	auto const trigChMask = channelMask(triggerChannel);
	hDevice.hCommand("setting channel self trigger", [trigChMask](int handle) {
		return CAEN_DGTZ_SetChannelSelfTrigger(
				handle,
				CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT,
				trigChMask
		);
	});

	triggerThreshold = odb::getValueUInt32V(hDB, hSet,
			settings::triggerThreshold, boardInfo.Channels,
			defaults::triggerThreshold, true);

	triggerRaisingPolarity = odb::getValueBoolV(hDB, hSet,
			settings::triggerRaisingPolarity, boardInfo.Channels,
			defaults::triggerRaisingPolarity, true);

	auto const chMask = trigChMask | channelMask(enabledChannels);
	hDevice.hCommand("setting channel enable mask", [chMask](int handle) {
		return CAEN_DGTZ_SetChannelEnableMask(handle, chMask);
	});

	for (std::size_t ch = 0; ch != enabledChannels.size(); ch++) {
		hDevice.hCommand("setting record length",
				[recordLength, ch](int handle) {
					return CAEN_DGTZ_SetRecordLength(handle, recordLength, ch);
				});

		if (dcOffsets.size() >= ch) {
			hDevice.hCommand("setting channel DC offset",
					[this, ch](int handle) {
						return CAEN_DGTZ_SetChannelDCOffset(handle, ch, dcOffsets[ch]);
					});
		}

		if (triggerThreshold.size() >= ch) {
			if (triggerThreshold[ch] > getMaxSampleValue()) {
				std::stringstream ss;
				ss << "Invalid value of " << settings::triggerThreshold << "["
						<< ch << "] parameter: " << triggerThreshold[ch]
						<< " is greater than max sample value ("
						<< getMaxSampleValue() << ")";
				throw midas::Exception(FE_ERR_ODB, ss.str());
			}

			hDevice.hCommand("setting channel trigger threshold",
					[this, ch](int handle) {
						return CAEN_DGTZ_SetChannelTriggerThreshold(handle, ch, triggerThreshold[ch]);
					});
		}

		if (triggerRaisingPolarity.size() > ch) {
			hDevice.hCommand("setting trigger polarity",
					[this, ch](int handle) {
						return CAEN_DGTZ_SetTriggerPolarity(
								handle,
								ch,
								triggerRaisingPolarity[ch] ? CAEN_DGTZ_TriggerOnRisingEdge : CAEN_DGTZ_TriggerOnFallingEdge
						);
					});
		}

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
				postTriggerRegisterValue(recordLength, preTriggerLength));
	}

	// disable trigger overlap
	hDevice.setBit(::caen::reg::BROAD_CH_CTRL_ADD,
			::caen::regbit::config::TRIGGER_OVERLAP, false);

	// make pattern field to have trigger source
	hDevice.setBit(::caen::reg::FRONT_PANEL_IO_CTRL_ADD, 21, true);
	hDevice.setBit(::caen::reg::FRONT_PANEL_IO_CTRL_ADD, 22, false);

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
	acquisitionIsOn = true;

}

void DigitizerFrontend::stopAcquisition(::caen::Device& device) {

	acquisitionIsOn = false;
	device.stopAcquisition();

}

void DigitizerFrontend::closeDevice() {

	if (device) {
		try {
			stopAcquisition(*device);
		} catch (::caen::Exception& e) {
			util::FrontEndUtils::handleCaenException(e);
		}

		device = nullptr;
	}

}

void DigitizerFrontend::storeTriggerBank(char* const pevent) {

	if (!channelMask(triggerChannel)) {
		return;	//	 no self-triggers
	}

	util::TriggerBank* bank;
	bk_create(pevent, util::TriggerInfoRawData::bankName(), TID_WORD,
			(void**) &bank);

	for (int8_t ch = 0; ch != static_cast<int8_t>(boardInfo.Channels); ch++) {
		if (triggerChannel[ch]) {
			util::fillTriggerInfo(*bank, ch, triggerThreshold[ch],
					triggerRaisingPolarity[ch], false);
			bank++;
		}
	}

	bk_close(pevent, bank);

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

void DigitizerFrontend::storeSignalInfoBank(char* pevent) {

	util::SignalInfoBank* pdata;
	bk_create(pevent, util::SignalInfoRawData::BANK_NAME, TID_DWORD,
			(void**) &pdata);
	for (unsigned i = 0; i < boardInfo.Channels; i++) {
		util::fillSignalInfo(*pdata, signalLengths[i], signalFrontLengths[i],
				signalTriggerChannel[i], signalRisingPolarities[i]);
		pdata++;
	}
	bk_close(pevent, pdata);

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
		info->timeStamp = eventInfo.TriggerTimeTag;
		info->pattern.raw = eventInfo.Pattern;
		info->frontendIndex = util::FrontEndUtils::frontendIndex<
				decltype(info->frontendIndex)>();
		info->preTriggerLength = preTriggerLength;
		info->triggerMode = 0;
		bk_close(pevent, pdata + sizeof(*info));
	}

	storeTriggerBank(pevent);
	storeDcOffsetBank(pevent);
	storeSignalInfoBank(pevent);
	storeWaveformBanks(pevent, eventInfo, event);

	return bk_size(pevent);

}

uint32_t DigitizerFrontend::channelMask(std::vector<bool> const& channelState) {

	uint32_t channelMask = 0;

	for (std::size_t i = 0; i != channelState.size(); i++) {
		if (channelState[i]) {
			channelMask |= 0x0001 << i;
		}
	}

	return channelMask;

}

}

}
