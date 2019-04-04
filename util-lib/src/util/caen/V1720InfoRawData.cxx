#include <util/caen/V1720InfoRawData.hxx>
#include <caen/v1720.hxx>

namespace util {

namespace caen {

static const char* const V1720InfoBankName = "V200";

V1720InfoRawData::V1720InfoRawData(int const bklen, int const bktype,
		const char* const name, void* const pdata) :
		DigitizerInfoRawData(bklen, bktype, name, pdata) {
}

const char* V1720InfoRawData::bankName() {

	return V1720InfoBankName;

}

uint64_t V1720InfoRawData::timeStampDifferenceInNs(
		timestamp_type const ts) const {

	return TimestampOp::sub(timeStamp(), ts)
			* ::caen::v1720::SAMPLES_PER_TIME_TICK
			* ::caen::v1720::nsPerSample();

}

}

}
