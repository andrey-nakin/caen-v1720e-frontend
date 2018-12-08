#ifndef	__frontend_util_lib_types_hxx__
#define	__frontend_util_lib_types_hxx__

#include <cstdint>

namespace util {

enum class DataType : uint32_t {
	WaveForm16bitVer1 = 1
};

enum class DeviceType : uint32_t {
	CaenV1720E = 1
};

typedef struct __attribute__((packed)) {

	DataType dataType;
	DeviceType deviceType;
	uint32_t boardId;
	uint32_t channelMask;
	uint32_t eventCounter;
	uint64_t timeStamp;
	uint32_t recordLength;
	uint32_t preTriggerLength;

} InfoBank;

}

#endif	//	__frontend_util_lib_types_hxx__
