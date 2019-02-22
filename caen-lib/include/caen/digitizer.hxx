#ifndef	__CAEN_DIGITIZER_HXX__
#define	__CAEN_DIGITIZER_HXX__

#include <cstdint>
#include <CAENDigitizer.h>

namespace caen {

namespace reg {

constexpr uint32_t FRONT_PANEL_IO_CTRL_ADD = CAEN_DGTZ_FRONT_PANEL_IO_CTRL_ADD;
constexpr uint32_t BROAD_CH_CTRL_ADD = CAEN_DGTZ_BROAD_CH_CTRL_ADD;
constexpr uint32_t POST_TRIG_ADD = CAEN_DGTZ_POST_TRIG_ADD;

}

namespace regbit {

/**
 * Port 0x8000
 * Board Configuration
 */
namespace config {

/**
 * Trigger Overlap Setting:
 * 0 = Trigger Overlapping Not Allowed;
 * 1 = Trigger Overlapping Allowed.
 */
constexpr uint8_t TRIGGER_OVERLAP = 1;

}

/**
 * Port 0x811C
 * Front Panel I/O Control
 */
namespace fpioctl {

/**
 * I/Os Electrical Level:
 * 0 = NIM I/O levels;
 * 1 = TTL I/O levels.
 */
constexpr uint8_t IO_LEVEL = 0;

}

}

}

#endif	//	__CAEN_DIGITIZER_HXX__

