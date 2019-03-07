#ifndef	__CAEN_V1724_HXX__
#define	__CAEN_V1724_HXX__

#include <cstdint>

namespace caen {

namespace v1724 {

constexpr uint8_t NUM_OF_CHANNELS = 8;

constexpr uint32_t MAX_RECORD_LENGTH = 512 * 1024;	//	samples

constexpr uint16_t MIN_SAMPLE_VALUE = 0;
constexpr uint16_t MAX_SAMPLE_VALUE = 16383;
constexpr uint16_t NUM_OF_SAMPLE_VALUES = MAX_SAMPLE_VALUE - MIN_SAMPLE_VALUE
		+ 1;

constexpr unsigned SAMPLES_PER_TIME_TICK = 2;

/**
 * Number of nanoseconds per one sample
 */
template<typename T>
constexpr T nsPerSample() {

	return 10;

}

}

}

#endif	//	__CAEN_V1724_HXX__

