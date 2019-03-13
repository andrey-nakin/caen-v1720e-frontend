#ifndef	UTIL_LIB_TriggerInfoRawData_hxx
#define	UTIL_LIB_TriggerInfoRawData_hxx

#include <TGenericData.hxx>
#include "types.hxx"

namespace util {

class TriggerInfoRawData: public TGenericData {
public:

	typedef uint8_t channelno_type;

	TriggerInfoRawData(int bklen, int bktype, const char* name, void *pdata);

	static const char* bankName();

	TriggerBank const* channelInfo(channelno_type channel) const;

	static channelno_type channel(TriggerBank const&);
	static uint16_t threshold(TriggerBank const&);
	static bool rising(TriggerBank const&);

private:

	int numOfChannels() const;
	TriggerBank const* info(int idx) const;

};

}

#endif	//	UTIL_LIB_TriggerInfoRawData_hxx
