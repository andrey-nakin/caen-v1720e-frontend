#pragma once

#include <string>
#include <vector>
#include <Converter.hxx>
#include <util/caen/DigitizerInfoRawData.hxx>

namespace gdc {

namespace converter {

class Binary: public Converter {

	static constexpr std::size_t waveformSize = 1000;
	static constexpr std::size_t waveformTail = 50;
	static constexpr std::size_t seriesSize = 100;
	static constexpr std::size_t seriesFillerSize = 23;

public:

	Binary();
	using Converter::Converter;

	static std::string Name();

	void ProcessMidasEvent(std::ostream& dest, TDataContainer& dataContainer)
			override;

	std::string FileExtension() const override;
	std::ios_base::openmode FileMode() const override;

private:

	std::vector<uint8_t> waveformFiller, seriesFiller;

	void ProcessMidasEvent(std::ostream& dest, TDataContainer& dataContainer,
			util::caen::DigitizerInfoRawData const& info, int bitmove);
	void WriteWaveform(std::ostream& dest, TDataContainer& dataContainer,
			util::caen::DigitizerInfoRawData const& info, uint8_t channel,
			int bitmove);

};

}

}
