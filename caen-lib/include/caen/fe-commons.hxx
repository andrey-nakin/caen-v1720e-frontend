#ifndef	__CAEN_FE_COMMONS_HXX__
#define	__CAEN_FE_COMMONS_HXX__

#include <memory>
#include <mutex>
#include <atomic>
#include <vector>
#include <midas.h>
#include <CAENDigitizer.h>
#include <midas/odb.hxx>
#include <caen/device.hxx>
#include <caen/digitizer.hxx>
#include <util/FrontEndUtils.hxx>

#ifndef NEED_NO_EXTERN_C
extern "C" {
#endif

extern const char *frontend_name;
extern EQUIPMENT equipment[];
extern HNDLE hDB;

INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();
INT poll_event(INT source, INT count, BOOL test);
INT interrupt_configure(INT cmd, INT source, PTYPE adr);

#ifndef NEED_NO_EXTERN_C
}
#endif

namespace fe {

namespace commons {

constexpr uint32_t MAX_NUM_OF_EVENTS = 100;
constexpr int EVID = 1;

namespace glob {

extern CAEN_DGTZ_BoardInfo_t boardInfo;
extern std::unique_ptr<caen::Device> device;
extern std::recursive_mutex mDevice;
extern std::atomic_bool acquisitionIsOn;
extern uint32_t preTriggerLength;
extern std::vector<uint16_t> dcOffsets;
extern uint8_t triggerChannel;
extern uint16_t triggerThreshold;
extern bool triggerRaisingPolarity;

}

namespace iolevel {

constexpr char nim[] = "nim";
constexpr char ttl[] = "ttl";

}

namespace defaults {

constexpr int32_t linkNum = 0;
constexpr int32_t conetNode = 0;
constexpr uint32_t vmeBaseAddr = 0;
constexpr uint32_t preTriggerLength = 64;
constexpr bool extTrigger = false;
constexpr auto frontPanelIOLevel = iolevel::nim;
constexpr uint32_t recordLength = 1024;
constexpr uint8_t triggerChannel = 0;
constexpr uint16_t triggerThreshold = 1000;
constexpr bool triggerRaisingPolarity = false;

namespace channel {

constexpr bool enabled = true;
constexpr uint16_t dcOffset = 0x8000;

}

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

}

caen::Handle connect();
void configure(caen::Handle& hDevice, HNDLE hSet,
		caen::DigitizerDetails const&);
void startAcquisition(caen::Device& device);
void stopAcquisition(caen::Device& device);

void storeTriggerBank(char* pevent);
void storeDcOffsetBank(char* pevent);
void storeWaveformBanks(char* pevent, CAEN_DGTZ_EventInfo_t const& eventInfo, CAEN_DGTZ_UINT16_EVENT_t const& event);

void configureDevice(caen::Handle& hDevice);

}

}

#endif	//	__CAEN_FE_COMMONS_HXX__
