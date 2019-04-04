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

	return timeStampDifferenceInSamples(ts) * ::caen::v1720::nsPerSample();

}

uint64_t V1720InfoRawData::timeStampDifferenceInSamples(
		timestamp_type ts) const {

	return static_cast<uint64_t>(TimestampOp::sub(timeStamp(), ts))
			* ::caen::v1720::SAMPLES_PER_TIME_TICK;

}

uint8_t V1720InfoRawData::sampleWidthInBits() const {

	return ::caen::v1720::SAMPLE_WIDTH_IN_BITS;

}

}

}
