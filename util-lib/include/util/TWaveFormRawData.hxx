#ifndef	UTIL_LIB_TWaveFormRawData_hxx
#define	UTIL_LIB_TWaveFormRawData_hxx

#include <cstdint>
#include <TGenericData.hxx>
#include "types.hxx"

namespace util {

class TWaveFormRawData: public TGenericData {
public:

	TWaveFormRawData(int bklen, int bktype, const char* name, void *pdata);

	static const char* bankName(uint8_t channelNo);

	uint16_t const* waveForm() const {

		return GetData16();

	}

};

}

#endif	//	UTIL_LIB_TWaveFormRawData_hxx
