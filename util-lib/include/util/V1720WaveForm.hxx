#ifndef	UTIL_LIB_V1720WaveForm_hxx
#define	UTIL_LIB_V1720WaveForm_hxx

#include <cstdint>
#include <vector>
#include <RtypesCore.h>
#include "util/TWaveFormRawData.hxx"

namespace util {

class V1720WaveForm {
public:

	V1720WaveForm(TWaveFormRawData const& rawData, uint16_t dcOffset);

	std::size_t size() const {

		return voltages.size();

	}

	// return times in nanoseconds
	std::vector<Float_t> const& getTimes() const {

		return times;

	}

	Float_t const* getTimePtr() const {

		return &times[0];

	}

	// return values in volts
	std::vector<Float_t> const& getVoltages() const {

		return voltages;

	}

	Float_t const* getVoltagePtr() const {

		return &voltages[0];

	}

private:

	std::vector<Float_t> times, voltages;

};

}

#endif	//	UTIL_LIB_V1720WaveForm_hxx
