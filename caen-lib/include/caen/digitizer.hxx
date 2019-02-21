#ifndef	__CAEN_DIGITIZER_HXX__
#define	__CAEN_DIGITIZER_HXX__

#include <cstdint>

namespace caen {

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

