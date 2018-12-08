#ifndef	__CAEN_V1720_HXX__
#define	__CAEN_V1720_HXX__

#include <cstdint>
#include "noncopyable.hxx"
#include "error-holder.hxx"

namespace caen {

namespace v1720 {

constexpr uint32_t REG_CHANNEL_CONFIG = 0x8000;
constexpr uint32_t REG_POST_TRIGGER = 0x8114;

constexpr uint32_t REG_BIT_TRIGGER_OVERLAP = 0x0001 << 1;

constexpr unsigned TOTAL_MEMORY_SIZE = 10 * 1024 * 1024 * sizeof(uint16_t);

}

}

#endif	//	__CAEN_V1720_HXX__

