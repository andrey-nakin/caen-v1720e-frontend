#ifndef	UTIL_LIB_TDcOffsetRawData_hxx
#define	UTIL_LIB_TDcOffsetRawData_hxx

#include <cstdint>
#include <TGenericData.hxx>
#include "types.hxx"

namespace util {

class TDcOffsetRawData: public TGenericData {
public:

	typedef uint16_t value_type;

	static char const* BANK_NAME;

	TDcOffsetRawData(int bklen, int bktype, const char* name, void *pdata);

	value_type dcOffset(uint8_t channelNo) const;

};

}

#endif	//	UTIL_LIB_TDcOffsetRawData_hxx
