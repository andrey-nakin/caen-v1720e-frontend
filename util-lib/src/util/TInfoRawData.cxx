#include <util/TInfoRawData.hxx>

namespace util {

char const* TInfoRawData::BANK_NAME = "INFO";

TInfoRawData::TInfoRawData(int const bklen, int const bktype,
		const char* const name, void * const pdata) :
		TGenericData(bklen, bktype, name, pdata) {

}

InfoBank const& TInfoRawData::info() const {

	return *reinterpret_cast<InfoBank const*>(GetData32());

}

}
