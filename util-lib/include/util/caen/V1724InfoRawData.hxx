#pragma once

#include <util/caen/DigitizerInfoRawData.hxx>

namespace util {

namespace caen {

extern const char* V1724InfoBankName;

class V1724InfoRawData: public DigitizerInfoRawData<&V1724InfoBankName> {
public:

	V1724InfoRawData(int bklen, int bktype, const char* name, void *pdata);

};

}

}
