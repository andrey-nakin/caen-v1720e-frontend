#include <util/caen/V1720InfoRawData.hxx>

namespace util {

namespace caen {

const char* V1720InfoBankName = "V200";

V1720InfoRawData::V1720InfoRawData(int const bklen, int const bktype,
		const char* const name, void* const pdata) :
		DigitizerInfoRawData<&V1720InfoBankName>(bklen, bktype, name, pdata) {
}

}

}
