#pragma once

#include <util/caen/DigitizerInfoRawData.hxx>

namespace util {

namespace caen {

extern const char* V1720InfoBankName;

class V1720InfoRawData: public DigitizerInfoRawData<&V1720InfoBankName> {
public:

	V1720InfoRawData(int bklen, int bktype, const char* name, void *pdata);

};

}

}
