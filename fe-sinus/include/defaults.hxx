#ifndef	__fe_v1720_defaults_cxx__
#define	__fe_v1720_defaults_cxx__

#include <cstdint>
#include <string>

namespace defaults {

constexpr uint8_t numOfChannels = 8;
constexpr uint32_t recordLength = 1024;
constexpr uint32_t dFrequency = 250000000;

namespace channel {

constexpr bool enabled = true;
constexpr uint16_t dcOffset = 0x8000;
constexpr uint16_t amplitude = 0x6000;
constexpr uint32_t frequency = 250000;
constexpr int32_t phase = 0;

}

}

#endif	//	__fe_v1720_defaults_cxx__
