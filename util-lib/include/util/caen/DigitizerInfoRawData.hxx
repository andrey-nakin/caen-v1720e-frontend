#pragma once

#include <cstring>
#include <TGenericData.hxx>
#include <util/types.hxx>
#include <math/IntOp.hxx>
#include <util/TDcOffsetRawData.hxx>

namespace util {

namespace caen {

class DigitizerInfoRawData: public TGenericData {
protected:

	typedef math::IntOp<uint32_t, 31> TimestampOp;

public:

	typedef TimestampOp::value_type timestamp_type;

	DigitizerInfoRawData(int bklen, int bktype, const char* name, void *pdata);
	DigitizerInfoRawData(DigitizerInfoRawData const&) = delete;
	DigitizerInfoRawData& operator=(DigitizerInfoRawData const&) = delete;

	InfoBank const& info() const;

	bool channelIncluded(uint8_t const channelNo) const;

	bool hasTriggerSettings() const;

	uint32_t preTriggerLength() const;

	uint32_t triggerMode() const;

	bool selfTrigger(uint8_t const channel) const;

	bool extTrigger() const;

	bool hasSelfTriggers() const;

	timestamp_type timeStamp() const;

	/**
	 * to_nanosecs(<this timestamp> - <ts>)
	 */
	virtual uint64_t timeStampDifferenceInNs(timestamp_type ts) const = 0;

	/**
	 * to_num_of_samples(<this timestamp> - <ts>)
	 */
	virtual uint64_t timeStampDifferenceInSamples(timestamp_type ts) const = 0;

	virtual uint8_t sampleWidthInBits() const = 0;

	int frontendIndex() const;

	virtual double dcMultiplier() const = 0;

	virtual double dcBaseline(TDcOffsetRawData::value_type dcOffset) const = 0;

private:

	InfoBank infoBank;

};

}

}
