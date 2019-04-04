#include <util/caen/DigitizerInfoRawData.hxx>

namespace util {

namespace caen {

DigitizerInfoRawData::DigitizerInfoRawData(int bklen, int bktype,
		const char* name, void *pdata) :
		TGenericData(bklen, bktype, name, pdata) {

	std::memset(&infoBank, 0, sizeof(infoBank));
	infoBank.frontendIndex = static_cast<decltype(infoBank.frontendIndex)>(-1);
	std::memcpy(&infoBank, GetData32(),
			std::min(sizeof(infoBank), GetSize() * sizeof(GetData32()[0])));

}

InfoBank const& DigitizerInfoRawData::info() const {

	return infoBank;

}

bool DigitizerInfoRawData::channelIncluded(uint8_t const channelNo) const {

	return (info().channelMask & (0x0001 << channelNo)) != 0;

}

bool DigitizerInfoRawData::hasTriggerSettings() const {

	return GetSize() >= 8;

}

uint32_t DigitizerInfoRawData::preTriggerLength() const {

	return info().preTriggerLength;

}

uint32_t DigitizerInfoRawData::triggerMode() const {

	return info().triggerMode;

}

bool DigitizerInfoRawData::selfTrigger(uint8_t const channel) const {

	return 0 != (info().pattern.bits.channelTrigger & (0x01 << channel));

}

bool DigitizerInfoRawData::extTrigger() const {

	return 0 != info().pattern.bits.extTrigger;

}

bool DigitizerInfoRawData::hasSelfTriggers() const {

	return info().pattern.bits.channelTrigger != 0;

}

DigitizerInfoRawData::timestamp_type DigitizerInfoRawData::timeStamp() const {

	return info().timeStamp;

}

int DigitizerInfoRawData::frontendIndex() const {

	return static_cast<int>(info().frontendIndex);

}

}

}
