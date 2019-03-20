#pragma once

#include <map>
#include "AbstractWaveform.hxx"
#include <util/TWaveFormRawData.hxx>
#include <util/TriggerInfoRawData.hxx>
#include <util/SignalInfoRawData.hxx>
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

	typedef math::IntOp<uint32_t, 31> TimestampOp;
	typedef uint8_t channel_no_type;
	typedef uint32_t wflength_type;
	typedef ::util::TWaveFormRawData::difference_type distance_type;

	static constexpr bool DEF_SIGNAL_RISING = false;
	static constexpr wflength_type DEF_SIGNAL_LENGTH = 16;
	static constexpr wflength_type DEF_SIGNAL_FRONT_LENGTH = 3;
	static constexpr double DEF_SIGNAL_THRESHOLD_KAPPA = 7.0;
	static constexpr channel_no_type EXT_TRIGGER =
			static_cast<channel_no_type>(-1);
	static constexpr wflength_type MAX_POSITION = 1000000;

	DigitizerWaveform(VirtualOdb* anOdb, std::string const& anOdbRootKey,
			std::string const& aBaseEquipName, std::string const & aDisplayName,
			ns_per_sample_type const aNsPerSample);

	void UpdateHistograms(TDataContainer &dataContainer,
			::util::caen::DigitizerInfoRawData const& info);

private:

	std::map<channel_no_type, distance_type> triggers;
	std::map<channel_no_type, TimestampOp::value_type> triggerTimestamps;

	virtual unsigned samplesPerTimeTick() const = 0;
	virtual channel_no_type numOfChannels() const = 0;
	virtual unsigned numOfSampleValues() const = 0;
	virtual uint16_t maxSampleValue() const = 0;

	channel_no_type CurrentTrigger(
			::util::caen::DigitizerInfoRawData const& info) const;

	std::pair<bool, ::util::TWaveFormRawData::value_type> HasPeaks(
			::util::TWaveFormRawData::const_iterator_type wfBegin,
			::util::TWaveFormRawData::const_iterator_type wfEnd,
			::util::SignalInfoBank const* signalInfo, wflength_type frontLength,
			bool rising) const;

	std::pair<bool, distance_type> CalcPosition(
			::util::caen::DigitizerInfoRawData const& info, distance_type wfPos,
			::util::SignalInfoBank const* signalInfo);

	void AnalyzeWaveform(::util::caen::DigitizerInfoRawData const& info,
			channel_no_type channelNo,
			::util::TWaveFormRawData::const_iterator_type wfBegin,
			::util::TWaveFormRawData::const_iterator_type wfEnd,
			::util::SignalInfoBank const* signalInfo);

	void DetectTrigger(TDataContainer &dataContainer,
			::util::caen::DigitizerInfoRawData const& info);

	void AddTimeAmplitude(::util::caen::DigitizerInfoRawData const& info,
			::util::TWaveFormRawData::const_iterator_type wfBegin,
			::util::TWaveFormRawData::const_iterator_type wfEnd, HistType& ph,
			HistType& ah, ::util::SignalInfoBank const* signalInfo,
			uint32_t preTriggerLength, uint32_t peakLength, bool rising,
			::util::TWaveFormRawData::const_iterator_type i);

	static TimestampOp::value_type timestamp(
			::util::caen::DigitizerInfoRawData const& info) {

		return TimestampOp::value(info.info().timeStamp);

	}

	static TimestampOp::value_type timestampDiff(
			TimestampOp::value_type const first,
			TimestampOp::value_type const last) {

		return TimestampOp::sub(last, first);

	}

	template<typename IntT>
	static INT frontendIndex(IntT const v) {

		return v == static_cast<decltype(v)>(-1) ? -1 : static_cast<INT>(v);

	}

};

}

}
