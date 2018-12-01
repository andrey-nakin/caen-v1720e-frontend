#ifndef	__fe_v1720_defaults_cxx__
#define	__fe_v1720_defaults_cxx__

#include <cstdint>
#include <string>

namespace defaults {

constexpr int32_t linkNum = 0;
constexpr int32_t conetNode = 0;
constexpr uint32_t vmeBaseAddr = 0;
constexpr uint32_t recordLength = 1024;
constexpr uint32_t preTriggerLength = 64;
constexpr uint8_t triggerChannel = 0;
constexpr uint32_t triggerThreshold = 1000;
constexpr bool triggerRaisingPolarity = false;
constexpr char triggerMode[] = "";

namespace channel {

constexpr bool enabled = true;
constexpr uint32_t dcOffset = 0x8000;

}

}

#endif	//	__fe_v1720_defaults_cxx__
