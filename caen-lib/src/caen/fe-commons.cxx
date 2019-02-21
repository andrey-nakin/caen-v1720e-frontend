#include <cctype>
#include <algorithm>
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

static std::string to_lower(std::string const& s) {

	std::string res = s;
	std::transform(res.begin(), res.end(), res.begin(), ::tolower);
	return res;

}

void configure(caen::Handle& hDevice, HNDLE const hSet) {

	// disable trigger overlap
	hDevice.setBit(CAEN_DGTZ_BROAD_CH_CTRL_ADD,
			caen::regbit::config::TRIGGER_OVERLAP, false);

	// set front panel IO level
	auto const fpIOLevel = odb::getValueString(hDB, hSet,
			settings::frontPanelIOLevel, defaults::frontPanelIOLevel, true, 32);
	hDevice.setBit(CAEN_DGTZ_FRONT_PANEL_IO_CTRL_ADD,
			caen::regbit::fpioctl::IO_LEVEL,
			to_lower(fpIOLevel) == IOLevel::ttl);

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
