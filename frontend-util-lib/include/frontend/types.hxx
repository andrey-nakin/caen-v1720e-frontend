#ifndef	__frontend_util_lib_types_hxx__
#define	__frontend_util_lib_types_hxx__

#include <cstdint>

namespace fe {

enum class DataType : uint32_t {
	WaveForm16bitVer1 = 1
};

enum class Device : uint32_t {
	CaenV1720E = 1
};

typedef struct __attribute__((packed)) {

	DataType dataType;
	Device device;
	uint32_t recordLength;
	uint32_t preTriggerLength;
	uint64_t timeStamp;

} InfoBank;

}

#endif	//	__frontend_util_lib_types_hxx__
