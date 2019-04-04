#pragma once

#include <util/caen/DigitizerInfoRawData.hxx>

namespace util {

namespace caen {

class V1724InfoRawData: public DigitizerInfoRawData {
public:

	V1724InfoRawData(int bklen, int bktype, const char* name, void *pdata);

	static const char* bankName();

	uint64_t timeStampDifferenceInNs(timestamp_type ts) const override;

};

}

}
