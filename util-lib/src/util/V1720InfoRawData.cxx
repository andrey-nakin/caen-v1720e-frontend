#include <cstring>
#include <util/V1720InfoRawData.hxx>

namespace util {

V1720InfoRawData::V1720InfoRawData(int const bklen, int const bktype,
		const char* const name, void * const pdata) :
		TGenericData(bklen, bktype, name, pdata) {

	std::memcpy(&infoBank, GetData32(),
			std::min(sizeof(infoBank), GetSize() * sizeof(GetData32()[0])));

}

const char* V1720InfoRawData::bankName() {

	return "V200";

}

InfoBank const& V1720InfoRawData::info() const {

	return infoBank;

}

bool V1720InfoRawData::channelIncluded(uint8_t const channelNo) const {

	return (info().channelMask & (0x0001 << channelNo)) != 0;

}

bool V1720InfoRawData::hasTriggerSettings() const {

	return GetSize() >= 9;

}

uint8_t V1720InfoRawData::triggerChannel() const {

	return static_cast<uint8_t>(info().triggerChannel);

}

uint16_t V1720InfoRawData::triggerThreshold() const {

	return static_cast<uint16_t>(info().triggerThreshold);

}

bool V1720InfoRawData::triggerRising() const {

	return info().triggerRising == 0 ? false : true;

}

}
