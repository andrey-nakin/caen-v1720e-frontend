#include "caen/fe-commons.hxx"
#include "caen/digitizer.hxx"

INT frontend_init() {

	using namespace fe::commons;

	cm_msg(MDEBUG, frontend_name, "frontend_init");

	midas::Exception::setProgramName(frontend_name);

	util::FrontEndUtils::command([]() {

		odb::getValueInt32(hDB, 0,
				util::FrontEndUtils::settingsKeyName(equipment[0].name,
						"link_num"), defaults::linkNum, true);

		caen::Handle hDevice = connect();
		configureDevice(hDevice);

	});

	return SUCCESS;

}

INT frontend_exit() {

	using namespace fe::commons;

	cm_msg(MDEBUG, frontend_name, "frontend_exit");

	return util::FrontEndUtils::command([]() {

		std::lock_guard<decltype(glob::mDevice)> lock(glob::mDevice);

		if (glob::device) {
			stopAcquisition(*fe::commons::glob::device);
			glob::device = nullptr;
		}

	});

}

INT begin_of_run(INT run_number, char * /* error */) {

	using namespace fe::commons;

	cm_msg(MDEBUG, frontend_name, "begin_of_run run_number=%d", run_number);

	return util::FrontEndUtils::command([]() {

		std::lock_guard<decltype(glob::mDevice)> lock(glob::mDevice);

		glob::device.reset(new caen::Device(connect()));
		configureDevice(glob::device->getHandle());

		startAcquisition(*glob::device);

	});

}

INT end_of_run(INT run_number, char * /* error */) {

	using namespace fe::commons;

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

	using namespace fe::commons;

	cm_msg(MDEBUG, frontend_name, "pause_run run_number=%d", run_number);

	return util::FrontEndUtils::command([]() {

		std::lock_guard<decltype(glob::mDevice)> lock(glob::mDevice);

		stopAcquisition(*glob::device);

	});

}

INT resume_run(INT run_number, char * /* error */) {

	using namespace fe::commons;

	cm_msg(MDEBUG, frontend_name, "resume_run run_number=%d", run_number);

	return util::FrontEndUtils::command([]() {

		std::lock_guard<decltype(glob::mDevice)> lock(glob::mDevice);

		startAcquisition(*glob::device);

	});

}

INT frontend_loop() {

	return SUCCESS;

}

INT poll_event(INT /* source */, INT /* count */, BOOL const test) {

	using namespace fe::commons;

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

namespace fe {

namespace commons {

namespace glob {

CAEN_DGTZ_BoardInfo_t boardInfo;
std::unique_ptr<caen::Device> device;
std::recursive_mutex mDevice;
std::atomic_bool acquisitionIsOn(false);
uint32_t preTriggerLength;
std::vector<uint16_t> dcOffsets;
uint8_t triggerChannel;
uint16_t triggerThreshold;
bool triggerRaisingPolarity;

}

caen::Handle connect() {

	// save reference to settings tree
	auto const hSet = util::FrontEndUtils::settingsKey(equipment[0].name);

	auto const linkNum = odb::getValueInt32(hDB, hSet, settings::linkNum,
			defaults::linkNum, true);
	auto const conetNode = odb::getValueInt32(hDB, hSet, settings::conetNode,
			defaults::conetNode, true);
	auto const vmeBaseAddr = odb::getValueUInt32(hDB, hSet,
			settings::vmeBaseAddr, defaults::vmeBaseAddr, true);

	return caen::Handle(linkNum, conetNode, vmeBaseAddr);

}

void configure(caen::Handle& hDevice, HNDLE const hSet,
		caen::DigitizerDetails const& dig) {

	auto& boardInfo = glob::boardInfo;
	hDevice.hCommand("getting digitizer info", [&boardInfo](int handle) {
		return CAEN_DGTZ_GetInfo(handle, &boardInfo);
	});

	hDevice.hCommand("resetting digitizer", CAEN_DGTZ_Reset);

	hDevice.hCommand("setting run sync mode",
			[](int handle) {
				return CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled);
			});

	auto const recordLength = odb::getValueUInt32(hDB, hSet,
			settings::waveformLength, defaults::recordLength, true);
	if (recordLength > dig.maxRecordLength()) {
		throw midas::Exception(FE_ERR_ODB,
				std::string("Value of waveform_length parameter exceeds ")
						+ std::to_string(dig.maxRecordLength()));
	}

	auto const enabledChannels = odb::getValueBoolV(hDB, hSet,
			settings::enabledChannels, boardInfo.Channels,
			defaults::channel::enabled, true);

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

	hDevice.hCommand("setting channel enable mask", [channelMask](int handle) {
		return CAEN_DGTZ_SetChannelEnableMask(handle, channelMask);
	});

	auto const triggerThreshold = glob::triggerThreshold = odb::getValueUInt16(
			hDB, hSet, settings::triggerThreshold, defaults::triggerThreshold,
			true);
	hDevice.hCommand("setting channel trigger threshold",
			[triggerChannel, triggerThreshold](int handle) {
				return CAEN_DGTZ_SetChannelTriggerThreshold(handle, triggerChannel, triggerThreshold);
			});

	auto const triggerRaisingPolarity = glob::triggerRaisingPolarity =
			odb::getValueBool(hDB, hSet, settings::triggerRaisingPolarity,
					defaults::triggerRaisingPolarity, true);
	hDevice.hCommand("setting trigger polarity",
			[triggerChannel, triggerRaisingPolarity](int handle) {
				return CAEN_DGTZ_SetTriggerPolarity(handle, triggerChannel, triggerRaisingPolarity ? CAEN_DGTZ_TriggerOnRisingEdge : CAEN_DGTZ_TriggerOnFallingEdge);
			});

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
		auto const preTriggerLength = glob::preTriggerLength =
				odb::getValueUInt32(hDB, hSet, settings::preTriggerLength,
						defaults::preTriggerLength, true);
		if (preTriggerLength > recordLength) {
			throw midas::Exception(FE_ERR_ODB,
					std::string(
							"Invalid value of pre_trigger_length parameter: ")
							+ std::to_string(preTriggerLength)
							+ " is greater than wave form length ("
							+ std::to_string(recordLength) + " samples)");
		}

		hDevice.writeRegister(caen::reg::POST_TRIG_ADD,
				(recordLength - preTriggerLength) / 4);
	}

	// disable trigger overlap
	hDevice.setBit(caen::reg::BROAD_CH_CTRL_ADD,
			caen::regbit::config::TRIGGER_OVERLAP, false);

	// set front panel IO level
	{
		static std::vector<std::string> const IO_LEVELS = { iolevel::nim,
				iolevel::ttl };
		auto const v = odb::getValueString(hDB, hSet,
				settings::frontPanelIOLevel, IO_LEVELS);
		hDevice.setBit(caen::reg::FRONT_PANEL_IO_CTRL_ADD,
				caen::regbit::fpioctl::IO_LEVEL, v == iolevel::ttl);
	}

	hDevice.hCommand("setting max num events", [](int handle) {
		return CAEN_DGTZ_SetMaxNumEventsBLT(handle, MAX_NUM_OF_EVENTS);
	});

	hDevice.hCommand("setting acquisition mode", [](int handle) {
		return CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
	});

}

void startAcquisition(caen::Device& device) {

	device.startAcquisition();
	glob::acquisitionIsOn.store(true);

}

void stopAcquisition(caen::Device& device) {

	glob::acquisitionIsOn.store(false);
	device.stopAcquisition();

}

}

}
