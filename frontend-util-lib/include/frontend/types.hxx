#ifndef	__frontend_util_lib_types_hxx__
#define	__frontend_util_lib_types_hxx__

#include <cstdint>

namespace fe {

namespace ver1 {

typedef struct __attribute__ ((packed)) {

	uint64_t timeStamp;
	uint16_t dcOffset;
	uint32_t sampleTime;
	uint32_t numOfSamples;
	uint32_t preTrigger;
	uint16_t reserved;

} WaveForm16BitHeader;

typedef struct __attribute__ ((packed)) {

	uint16_t samples[];

} WaveForm16Bit;

}

enum class DataType : uint8_t {
	WaveForm
};

enum class DataVersion : uint8_t {
	Version1
};

enum class Device : uint8_t {
	CaenV1720E
};

typedef struct __attribute__ ((packed)) {

	uint16_t headerSize;
	DataType dataType;
	DataVersion dataVersion;
	Device device;
	uint8_t reserved[3];

	union {
		uint8_t rawData[];
		ver1::WaveForm16BitHeader waveForm16BitVer1;
	} customHeader;

	union {
		uint8_t rawData[];
		ver1::WaveForm16Bit waveForm16BitVer1;
	} data;

} BankType;

}

#endif	//	__frontend_util_lib_types_hxx__
