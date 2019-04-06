#include <util/TDcOffsetRawData.hxx>

namespace util {

char const* TDcOffsetRawData::BANK_NAME = "CHDC";

TDcOffsetRawData::TDcOffsetRawData(int const bklen, int const bktype,
		const char* const name, void * const pdata) :
		TGenericData(bklen, bktype, name, pdata) {

}

TDcOffsetRawData::value_type TDcOffsetRawData::dcOffset(
		uint8_t const channelNo) const {

	return channelNo < GetSize() ? GetData16()[channelNo] : 0;

}

}
