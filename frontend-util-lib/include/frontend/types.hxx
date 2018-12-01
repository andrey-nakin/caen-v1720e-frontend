#ifndef	__frontend_util_lib_types_hxx__
#define	__frontend_util_lib_types_hxx__

#include <cstdint>

namespace fe {

typedef struct __attribute__ ((packed)) {

	uint64_t timeStamp;
	uint16_t dcOffset;
	uint32_t sampleTime;
	uint32_t numOfSamples;
	uint32_t preTrigger;
	uint16_t reserved;

} WaveForm16BitVer1Header;

typedef struct __attribute__ ((packed)) {

	uint16_t samples[];

} WaveForm16BitVer1;

enum class DataType : uint8_t {
	WaveForm16bitVer1 = 1
};

enum class Device : uint8_t {
	CaenV1720E = 1
};

typedef struct __attribute__ ((packed)) {

	uint16_t headerSize;
	DataType dataType;
	Device device;
	uint8_t reserved[4];

	union {
		uint8_t rawData[];
		WaveForm16BitVer1Header waveForm16BitVer1;
	} customHeader;

	union {
		uint8_t rawData[];
		WaveForm16BitVer1 waveForm16BitVer1;
	} data;

} BankType;

}

#endif	//	__frontend_util_lib_types_hxx__
