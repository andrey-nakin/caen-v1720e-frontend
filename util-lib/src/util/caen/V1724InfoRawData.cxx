#include <util/caen/V1724InfoRawData.hxx>
#include <caen/v1724.hxx>

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

uint64_t V1724InfoRawData::timeStampDifferenceInNs(
		timestamp_type const ts) const {

	return TimestampOp::sub(timeStamp(), ts)
			* ::caen::v1724::SAMPLES_PER_TIME_TICK
			* ::caen::v1724::nsPerSample();

}

}

}
