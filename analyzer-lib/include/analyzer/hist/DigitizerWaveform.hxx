#pragma once

#include "AbstractWaveform.hxx"
#include <util/TWaveFormRawData.hxx>
#include <util/TriggerInfoRawData.hxx>
#include <util/caen/DigitizerInfoRawData.hxx>
#include <math/EdgeFinder.hxx>
#include <math/DiffContainer.hxx>
#include <math/StatAccum.hxx>
#include <math/PeakFinder.hxx>
#include <math/IntOp.hxx>

namespace analyzer {

namespace hist {

class DigitizerWaveform: public AbstractWaveform {
protected:

	typedef math::IntOp<uint32_t, 31> TimeStampOp;

	DigitizerWaveform(VirtualOdb* anOdb, std::string const& aBaseEquipName,
			std::string const & aDisplayName,
			ns_per_sample_type const aNsPerSample);

	void UpdateHistograms(TDataContainer &dataContainer,
			util::caen::DigitizerInfoRawData const* info);

	void AnalyzeWaveform(util::caen::DigitizerInfoRawData const* info,
			uint8_t channelNo, std::size_t numOfSamples,
			util::TWaveFormRawData::difference_type edgePosition,
			util::TWaveFormRawData::const_iterator_type wfBegin,
			util::TWaveFormRawData::const_iterator_type wfEnd);

	util::TWaveFormRawData::difference_type FindEdgeDistance(
			TDataContainer &dataContainer,
			util::caen::DigitizerInfoRawData const* info);

private:

	uint16_t minFront;
	unsigned frontLength, peakLength;
	double threshold;
	bool rising;
	bool masterEventOccurred;
	util::TWaveFormRawData::difference_type lastMasterEdgeDistance;
	util::InfoBank lastMasterEvent;

	virtual unsigned samplesPerTimeTick() const = 0;
	virtual unsigned numOfChannels() const = 0;
	virtual unsigned numOfSampleValues() const = 0;
	virtual uint16_t maxSampleValue() const = 0;

	static uint32_t timeStamp(util::InfoBank const& info) {

		return TimeStampOp::value(info.timeStamp);

	}

	static uint32_t timeDiff(uint32_t const first, uint32_t const last) {

		return TimeStampOp::sub(last, first);

	}

	template<typename IntT>
	static INT frontendIndex(IntT const v) {

		return v == static_cast<decltype(v)>(-1) ? -1 : static_cast<INT>(v);

	}

};

}

}
