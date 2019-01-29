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

	bool hasTriggerSettings() const;
	uint8_t triggerChannel() const;
	uint16_t triggerThreshold() const;
	bool triggerRising() const;
	uint32_t triggerMode() const;

private:

	InfoBank infoBank;

};

}

#endif	//	UTIL_LIB_V1720InfoRawData_hxx
