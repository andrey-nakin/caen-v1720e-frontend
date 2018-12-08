#include <util/TWaveFormRawData.hxx>

namespace util {

static const char* const bankNames[] = { "WF00", "WF01", "WF02", "WF03", "WF04",
		"WF05", "WF06", "WF07" };

template<typename T>
static std::string toString(T const v, std::size_t const len) {

	auto s = std::to_string(v);
	while (s.size() < len) {
		s = "0" + s;
	}
	return s;

}

TWaveFormRawData::TWaveFormRawData(int const bklen, int const bktype,
		const char* const name, void * const pdata) :
		TGenericData(bklen, bktype, name, pdata) {

}

const char* TWaveFormRawData::bankName(uint8_t const channelNo) {

	return channelNo < sizeof(bankNames) / sizeof(bankNames[0]) ?
			bankNames[channelNo] : nullptr;

}

}
