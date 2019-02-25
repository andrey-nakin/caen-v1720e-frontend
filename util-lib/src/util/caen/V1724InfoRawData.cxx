#include <util/caen/V1724InfoRawData.hxx>

namespace util {

namespace caen {

const char* V1724InfoBankName = "V240";

V1724InfoRawData::V1724InfoRawData(int const bklen, int const bktype,
		const char* const name, void* const pdata) :
		DigitizerInfoRawData<&V1724InfoBankName>(bklen, bktype, name, pdata) {
}

}

}
