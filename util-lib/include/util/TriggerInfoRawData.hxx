#ifndef	UTIL_LIB_TriggerInfoRawData_hxx
#define	UTIL_LIB_TriggerInfoRawData_hxx

#include <TGenericData.hxx>
#include "types.hxx"

namespace util {

class TriggerInfoRawData: public TGenericData {
public:

	TriggerInfoRawData(int bklen, int bktype, const char* name, void *pdata);

	static const char* bankName();

	uint8_t numOfChannels() const;
	int8_t masterTriggerChannel() const;

	TriggerBank const& info(uint8_t triggerChannel) const;

	uint8_t triggerChannel(uint8_t triggerChannel) const;
	uint16_t triggerThreshold(uint8_t triggerChannel) const;
	bool triggerRising(uint8_t triggerChannel) const;
	bool masterTrigger(uint8_t triggerChannel) const;

};

}

#endif	//	UTIL_LIB_TriggerInfoRawData_hxx
