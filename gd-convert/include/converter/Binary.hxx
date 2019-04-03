#pragma once

#include <string>
#include <vector>
#include <Converter.hxx>
#include <util/caen/DigitizerInfoRawData.hxx>
#include <math/IntOp.hxx>

namespace gdc {

namespace converter {

class Binary: public Converter {

	static constexpr std::size_t waveformSize = 1000;
	static constexpr std::size_t waveformTail = 50;
	static constexpr std::size_t seriesSize = 100;
	static constexpr std::size_t seriesFillerSize = 23;
	static constexpr uint64_t picosecsInANanoSec = 1000;

	typedef math::IntOp<uint32_t, 31> TimestampOp;

public:

	Binary();
	using Converter::Converter;

	static std::string Name();

	void ProcessMidasEvent(std::ostream& dest, TDataContainer& dataContainer)
			override;

	std::string FileExtension() const override;
	std::ios_base::openmode FileMode() const override;

private:

	unsigned eventCounter;
	uint32_t seriesStartTimeStamp;
	std::vector<uint8_t> waveformFiller, seriesFiller;

	void ProcessMidasEvent(std::ostream& dest, TDataContainer& dataContainer,
			util::caen::DigitizerInfoRawData const& info, int bitmove);
	void WriteWaveform(std::ostream& dest, TDataContainer& dataContainer,
			util::caen::DigitizerInfoRawData const& info, uint8_t channel,
			int bitmove);
	uint64_t CalcTimestamp(util::caen::DigitizerInfoRawData const& info);

};

}

}
