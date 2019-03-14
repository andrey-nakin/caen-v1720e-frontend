#pragma once

#include <atomic>
#include <memory>
#include <cstdint>
#include <vector>
#include <CAENDigitizer.h>
#include <caen/device.hxx>
#include <caen/digitizer.hxx>
#include "fe/SynchronizedFrontend.hxx"

namespace fe {

namespace caen {

constexpr uint32_t MAX_NUM_OF_EVENTS = 100;
constexpr int EVID = 1;

namespace defaults {

constexpr int32_t linkNum = 0;
constexpr int32_t conetNode = 0;
constexpr uint32_t vmeBaseAddr = 0;
constexpr uint32_t preTriggerLength = 64;
constexpr bool extTrigger = false;
constexpr uint32_t recordLength = 1024;
constexpr bool triggerChannel = false;
constexpr uint32_t triggerThreshold = 1000;
constexpr bool triggerRaisingPolarity = false;

namespace channel {

constexpr bool enabled = true;
constexpr uint16_t dcOffset = 0x8000;
constexpr uint32_t signalFrontLength = 3;
constexpr uint32_t signalLength = 16;
constexpr bool signalRisingPolarity = false;
constexpr uint8_t signalTriggerChannel = 0;
constexpr int16_t signalThreshold = -7;
constexpr uint32_t signalMaxTime = 1000000;

}

}

namespace iolevel {

constexpr char nim[] = "nim";
constexpr char ttl[] = "ttl";

}

namespace ext_trigger {

constexpr char off[] = "n";
constexpr char on[] = "y";
constexpr char gate[] = "gate";

}

namespace settings {

constexpr char linkNum[] = "link_num";
constexpr char conetNode[] = "conet_node";
constexpr char vmeBaseAddr[] = "vme_base_addr";
constexpr char extTrigger[] = "external_trigger";
constexpr char waveformLength[] = "waveform_length";
constexpr char preTriggerLength[] = "pre_trigger_length";
constexpr char frontPanelIOLevel[] = "front_panel_io_level";
constexpr char enabledChannels[] = "channel_enabled";
constexpr char channelDcOffset[] = "channel_dc_offset";
constexpr char triggerChannel[] = "trigger_channel";
constexpr char triggerThreshold[] = "trigger_threshold";
constexpr char triggerRaisingPolarity[] = "trigger_raising_polarity";
constexpr char signalTriggerChannel[] = "signal_trigger_channel";
constexpr char signalFrontLength[] = "signal_front_length";
constexpr char signalLength[] = "signal_length";
constexpr char signalRisingPolarity[] = "signal_rising_polarity";
constexpr char signalThreshold[] = "signal_threshold";
constexpr char signalMaxTime[] = "signal_max_time";

}

class DigitizerFrontend: public SynchronizedFrontend {
protected:

	DigitizerFrontend();

	std::atomic_bool acquisitionIsOn;
	std::unique_ptr<::caen::Device> device;
	CAEN_DGTZ_BoardInfo_t boardInfo;
	uint32_t preTriggerLength;
	std::vector<uint16_t> dcOffsets;
	std::vector<bool> triggerChannel;
	std::vector<uint32_t> triggerThreshold;
	std::vector<bool> triggerRaisingPolarity;
	std::vector<uint8_t> signalTriggerChannel;
	std::vector<uint32_t> signalFrontLengths;
	std::vector<uint32_t> signalLengths;
	std::vector<bool> signalRisingPolarities;
	std::vector<int16_t> signalThresholds;
	std::vector<uint32_t> signalMaxTimes;

	virtual uint32_t getMaxRecordLength() const = 0;
	virtual uint32_t getMaxSampleValue() const = 0;
	virtual char const* infoBankName() const = 0;
	virtual std::size_t calculateEventSize(unsigned numOfActiveChannels,
			unsigned recordLength) const = 0;
	virtual uint32_t postTriggerRegisterValue(uint32_t recordLength,
			uint32_t preTriggerLength) const = 0;

	virtual std::size_t calculateEventSize(
			CAEN_DGTZ_EventInfo_t const& eventInfo,
			CAEN_DGTZ_UINT16_EVENT_t const& event) const;

	static uint32_t channelMask(std::vector<bool> const& channelState);

private:

	void doInitSynchronized() override;
	void doExitSynchronized() override;
	void doBeginOfRunSynchronized(INT run_number, char* error) override;
	void doEndOfRunSynchronized(INT run_number, char* error) override;
	void doPauseRunSynchronized(INT run_number, char* error) override;
	void doResumeRunSynchronized(INT run_number, char* error) override;
	int doPollSynchronized() override;
	int doReadEventSynchronized(char* pevent, int off) override;
	void doOnStopSynchronized(INT run_number, char* error) override;

	virtual ::caen::Handle connect();
	virtual void checkBoardInfo(CAEN_DGTZ_BoardInfo_t const& boardInfo);
	virtual void configure(::caen::Handle& hDevice);

	virtual void startAcquisition(::caen::Device& device);
	virtual void stopAcquisition(::caen::Device& device);
	virtual void closeDevice();

	virtual void storeTriggerBank(char* pevent);
	virtual void storeDcOffsetBank(char* pevent);
	virtual void storeWaveformBanks(char* pevent,
			CAEN_DGTZ_EventInfo_t const& eventInfo,
			CAEN_DGTZ_UINT16_EVENT_t const& event);
	virtual void storeSignalInfoBank(char* pevent);

	virtual int parseEvent(char* pevent, CAEN_DGTZ_EventInfo_t const& eventInfo,
			CAEN_DGTZ_UINT16_EVENT_t const& event);

};

}

}
