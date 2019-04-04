#include <cstdint>
#include <converter/Binary.hxx>
#include <util/caen/V1720InfoRawData.hxx>
#include <util/caen/V1724InfoRawData.hxx>
#include <util/TWaveFormRawData.hxx>
#include <math/StatAccum.hxx>
#include <caen/v1720.hxx>
#include <caen/v1724.hxx>

namespace gdc {

namespace converter {

Binary::Binary() :
		eventCounter(0), seriesStartTimeStamp(0) {

	waveformFiller.resize(waveformSize);
	seriesFiller.resize(seriesFillerSize);

}

std::string Binary::Name() {

	return "binary";

}

void Binary::ProcessMidasEvent(std::ostream& dest,
		TDataContainer & dataContainer) {

	{
		using util::caen::V1720InfoRawData;

		auto const v1720Info = dataContainer.GetEventData < V1720InfoRawData
				> (V1720InfoRawData::bankName());
		if (v1720Info) {
			ProcessMidasEvent(dest, dataContainer, *v1720Info, 4,
					caen::v1720::nsPerSample()
							* caen::v1720::SAMPLES_PER_TIME_TICK);
			return;
		}
	}

	{
		using util::caen::V1724InfoRawData;

		auto const v1724Info = dataContainer.GetEventData < V1724InfoRawData
				> (V1724InfoRawData::bankName());
		if (v1724Info) {
			ProcessMidasEvent(dest, dataContainer, *v1724Info, 6,
					caen::v1724::nsPerSample()
							* caen::v1724::SAMPLES_PER_TIME_TICK);
			return;
		}
	}

}

std::string Binary::FileExtension() const {

	return ".data";

}

std::ios_base::openmode Binary::FileMode() const {

	return std::ios_base::out | std::ios::binary;

}

void Binary::ProcessMidasEvent(std::ostream& dest,
		TDataContainer& dataContainer,
		util::caen::DigitizerInfoRawData const& info, int const bitmove,
		int const tickToNs) {

	static uint32_t indexFirstPoint = 0;
	static double horPos = 0;
	uint64_t const timeStamp = CalcTimestamp(info, tickToNs);
	static uint16_t filler1 = 1;
	static uint64_t waveformLength = waveformSize + 1;
	static uint8_t waveformEnd = 0;

	for (uint8_t channel = 1; channel <= 8; channel++) {
		dest.write((char*) &channel, sizeof(channel));
		dest.write((char*) &indexFirstPoint, sizeof(indexFirstPoint));
		dest.write((char*) &horPos, sizeof(horPos));
		dest.write((char*) &timeStamp, sizeof(timeStamp));
		dest.write((char*) &filler1, sizeof(filler1));
		dest.write((char*) &waveformLength, sizeof(waveformLength));
		WriteWaveform(dest, dataContainer, info, channel, bitmove);
		dest.write((char*) &waveformEnd, sizeof(waveformEnd));
	}

	if (++eventCounter % seriesSize == 0) {
		dest.write((char*) &seriesFiller[0], seriesFiller.size());
		seriesStartTimeStamp = 0;
	}

}

void Binary::WriteWaveform(std::ostream& dest, TDataContainer& dataContainer,
		util::caen::DigitizerInfoRawData const& info, uint8_t const channel,
		int const bitmove) {

	using util::TWaveFormRawData;

	if (info.channelIncluded(channel - 1)) {
		auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
				> (TWaveFormRawData::bankName(channel - 1));
		if (wfRaw) {
			auto const numOfSamples = wfRaw->numOfSamples();
			if (numOfSamples > 0) {
				auto const wfBegin = wfRaw->begin();
				auto const wfEnd = wfRaw->end();
				auto const wfLast =
						numOfSamples <= waveformSize ?
								wfEnd : std::next(wfBegin, waveformSize);

				std::for_each(wfBegin, wfLast,
						[&dest, bitmove](TWaveFormRawData::value_type sample) {
							uint8_t const b = sample >> bitmove;
							dest.write((char*) &b, sizeof(b));
						});

				if (numOfSamples < waveformSize) {
					auto const tailBegin =
							numOfSamples >= waveformTail ?
									std::next(wfEnd, -waveformTail) : wfBegin;
					auto const zeroLevel =
							math::MakeStatAccum(tailBegin, wfEnd).GetRoughMean();
					char tailBuf[waveformSize];
					auto const tailSize = waveformSize - numOfSamples;
					std::memset(tailBuf, zeroLevel >> bitmove, tailSize);
					dest.write(tailBuf, tailSize);
				}

				return;
			}
		}
	}

	dest.write((char*) &waveformFiller[0], waveformFiller.size());

}

uint64_t Binary::CalcTimestamp(util::caen::DigitizerInfoRawData const& info,
		int const tickToNs) {

	if (seriesStartTimeStamp == 0) {
		seriesStartTimeStamp = TimestampOp::value(info.info().timeStamp);
		return 0;
	} else {
		return picosecsInANanoSec * tickToNs
				* TimestampOp::sub(info.info().timeStamp, seriesStartTimeStamp);
	}

}

}

}
