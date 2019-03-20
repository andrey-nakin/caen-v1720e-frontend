#pragma once

#include <cstring>
#include <TGenericData.hxx>
#include <util/types.hxx>

namespace util {

namespace caen {

class DigitizerInfoRawData: public TGenericData {
public:

	DigitizerInfoRawData(int bklen, int bktype, const char* name, void *pdata);

	InfoBank const& info() const;

	bool channelIncluded(uint8_t const channelNo) const;

	bool hasTriggerSettings() const;

	uint32_t preTriggerLength() const;

	uint32_t triggerMode() const;

	bool selfTrigger(uint8_t const channel) const;

	bool extTrigger() const;

	bool hasSelfTriggers() const;

private:

	InfoBank infoBank;

};

}

}
