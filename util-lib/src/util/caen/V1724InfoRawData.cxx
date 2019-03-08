#include <util/caen/V1724InfoRawData.hxx>

namespace util {

namespace caen {

static const char* const V1724InfoBankName = "V240";

V1724InfoRawData::V1724InfoRawData(int const bklen, int const bktype,
		const char* const name, void* const pdata) :
		DigitizerInfoRawData(bklen, bktype, name, pdata) {
}

const char* V1724InfoRawData::bankName() {

	return V1724InfoBankName;

}

}

}
