#ifndef	UTIL_LIB_V1720InfoRawData_hxx
#define	UTIL_LIB_V1720InfoRawData_hxx

#include <TGenericData.hxx>
#include "types.hxx"

namespace util {

class V1720InfoRawData: public TGenericData {
public:

	V1720InfoRawData(int bklen, int bktype, const char* name, void *pdata);

	static const char* bankName();

	InfoBank const& info() const;

	bool channelIncluded(uint8_t channelNo) const;

};

}

#endif	//	UTIL_LIB_V1720InfoRawData_hxx
