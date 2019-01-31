#ifndef	UTIL_LIB_TriggerInfoRawData_hxx
#define	UTIL_LIB_TriggerInfoRawData_hxx

#include <TGenericData.hxx>
#include "types.hxx"

namespace util {

class TriggerInfoRawData: public TGenericData {
public:

	TriggerInfoRawData(int bklen, int bktype, const char* name, void *pdata);

	static const char* bankName();

	TriggerBank const& info() const;

	uint8_t triggerChannel() const;
	uint16_t triggerThreshold() const;
	bool triggerRising() const;

};

}

#endif	//	UTIL_LIB_TriggerInfoRawData_hxx
