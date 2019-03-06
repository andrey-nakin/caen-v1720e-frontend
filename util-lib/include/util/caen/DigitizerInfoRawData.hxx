#pragma once

#include <cstring>
#include <TGenericData.hxx>
#include <util/types.hxx>

namespace util {

namespace caen {

template<const char** BankName>
class DigitizerInfoRawData: public TGenericData {
public:

	DigitizerInfoRawData(int bklen, int bktype, const char* name, void *pdata) :
			TGenericData(bklen, bktype, name, pdata) {

		std::memset(&infoBank, 0, sizeof(infoBank));
		infoBank.frontendIndex =
				static_cast<decltype(infoBank.frontendIndex)>(-1);
		std::memcpy(&infoBank, GetData32(),
				std::min(sizeof(infoBank), GetSize() * sizeof(GetData32()[0])));

	}

	static const char* bankName() {

		return *BankName;

	}

	InfoBank const& info() const {

		return infoBank;

	}

	bool channelIncluded(uint8_t const channelNo) const {

		return (info().channelMask & (0x0001 << channelNo)) != 0;

	}

	bool hasTriggerSettings() const {

		return GetSize() >= 8;

	}

	uint32_t preTriggerLength() const {

		return info().preTriggerLength;

	}

	uint32_t triggerMode() const {

		return info().triggerMode;

	}

	bool selfTrigger(uint8_t const channel) const {

		return 0 != info().pattern.bits.channelTrigger & (0x01 << channel);

	}

	bool extTrigger() const {

		return 0 != info().pattern.bits.extTrigger;

	}

private:

	InfoBank infoBank;

};

}

}
