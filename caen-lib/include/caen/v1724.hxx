#ifndef	__CAEN_V1724_HXX__
#define	__CAEN_V1724_HXX__

#include <cstdint>
#include "digitizer.hxx"

namespace caen {

namespace v1724 {

constexpr uint8_t NUM_OF_CHANNELS = 8;

constexpr uint32_t MAX_RECORD_LENGTH = 512 * 1024;	//	samples

constexpr uint16_t MIN_SAMPLE_VALUE = 0;
constexpr uint16_t MAX_SAMPLE_VALUE = 16383;
constexpr uint16_t NUM_OF_SAMPLE_VALUES = MAX_SAMPLE_VALUE - MIN_SAMPLE_VALUE
		+ 1;

/**
 * Number of nanoseconds per one sample
 */
template<typename T>
constexpr T nsPerSample() {

	return 10;

}

class V1724Details: public DigitizerDetails {
public:

	uint32_t maxRecordLength() const override {

		return MAX_RECORD_LENGTH;

	}

};

}

}

#endif	//	__CAEN_V1724_HXX__

