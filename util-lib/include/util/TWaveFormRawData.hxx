#ifndef	UTIL_LIB_TWaveFormRawData_hxx
#define	UTIL_LIB_TWaveFormRawData_hxx

#include <cstdint>
#include <TGenericData.hxx>
#include "types.hxx"

namespace util {

class TWaveFormRawData: public TGenericData {
public:

	typedef uint16_t value_type;

	TWaveFormRawData(int bklen, int bktype, const char* name, void *pdata);

	static const char* bankName(uint8_t channelNo);

	std::size_t numOfSamples() const {

		return GetSize();

	}

	value_type const* waveForm() const {

		return GetData16();

	}

};

}

#endif	//	UTIL_LIB_TWaveFormRawData_hxx
