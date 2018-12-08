#ifndef	UTIL_LIB_TInfoRawData_hxx
#define	UTIL_LIB_TInfoRawData_hxx

#include <TGenericData.hxx>
#include "types.hxx"

namespace util {

class TInfoRawData: public TGenericData {
public:

	static char const* BANK_NAME;

	TInfoRawData(int bklen, int bktype, const char* name, void *pdata);

	InfoBank const& info() const;

};

}

#endif	//	UTIL_LIB_TInfoRawData_hxx
