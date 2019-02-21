#ifndef	__CAEN_FE_COMMONS_HXX__
#define	__CAEN_FE_COMMONS_HXX__

#include <memory>
#include <mutex>
#include <atomic>
#include <midas.h>
#include <CAENDigitizer.h>
#include <midas/odb.hxx>
#include <caen/device.hxx>
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

namespace glob {

extern CAEN_DGTZ_BoardInfo_t boardInfo;
extern std::unique_ptr<caen::Device> device;
extern std::recursive_mutex mDevice;
extern std::atomic_bool acquisitionIsOn;

}

namespace defaults {

constexpr int32_t linkNum = 0;
constexpr int32_t conetNode = 0;
constexpr uint32_t vmeBaseAddr = 0;

namespace channel {

constexpr bool enabled = true;

}

}

namespace settings {

constexpr char linkNum[] = "link_num";
constexpr char conetNode[] = "conet_node";
constexpr char vmeBaseAddr[] = "vme_base_addr";
constexpr char frontPanelIOLevel[] = "front_panel_io_level";

}

caen::Handle connect();
void configure(caen::Handle& hDevice, HNDLE hSet);
void startAcquisition(caen::Device& device);
void stopAcquisition(caen::Device& device);

void configureDevice(caen::Handle& hDevice);

}

}

#endif	//	__CAEN_FE_COMMONS_HXX__
