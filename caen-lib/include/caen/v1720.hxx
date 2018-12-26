#ifndef	__CAEN_V1720_HXX__
#define	__CAEN_V1720_HXX__

#include <cstdint>

namespace caen {

namespace v1720 {

constexpr uint8_t NUM_OF_CHANNELS = 8;

constexpr uint32_t MAX_RECORD_LENGTH = 1024 * 1024;	//	samples

constexpr uint32_t REG_CHANNEL_CONFIG = 0x8000;
constexpr uint32_t REG_POST_TRIGGER = 0x8114;

constexpr uint32_t REG_BIT_TRIGGER_OVERLAP = 0x0001 << 1;

}

}

#endif	//	__CAEN_V1720_HXX__

