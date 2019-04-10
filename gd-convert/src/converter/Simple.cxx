#include <sstream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <util/caen/V1720InfoRawData.hxx>
#include <util/caen/V1724InfoRawData.hxx>
#include <util/TWaveFormRawData.hxx>
#include <util/TriggerInfoRawData.hxx>
#include <util/TDcOffsetRawData.hxx>
#include <math/EdgeFinder.hxx>
#include <converter/Simple.hxx>

namespace gdc {

namespace converter {

namespace cmd {

constexpr char triggerMask[] = "-tm";

}

static bool StartsWith(const char* const s, const char* const substr) {

	return s == std::strstr(s, substr);

}

Simple::Simple() :
		triggerMask(0xffff) {

}

std::string Simple::Name() {

	return "simple";

}

std::string Simple::FileExtension() const {

	return ".txt";

}

bool Simple::ProcessMidasEvent(std::ostream& dest,
		TDataContainer & dataContainer) {

	{
		using util::caen::V1720InfoRawData;

		auto const v1720Info = dataContainer.GetEventData < V1720InfoRawData
				> (V1720InfoRawData::bankName());
		if (v1720Info) {
			return ProcessMidasEvent(dest, dataContainer, *v1720Info);
		}
	}

	{
		using util::caen::V1724InfoRawData;

		auto const v1724Info = dataContainer.GetEventData < V1724InfoRawData
				> (V1724InfoRawData::bankName());
		if (v1724Info) {
			return ProcessMidasEvent(dest, dataContainer, *v1724Info);
		}
	}

	return true;

}

void Simple::Configure(std::vector<char*>& args) {

	for (std::size_t i = 0; i < args.size();) {
		if (StartsWith(args[i], cmd::triggerMask)) {
			triggerMask = std::stoi(args[i] + std::strlen(cmd::triggerMask));
		} else {
			i++;
			continue;
		}
		args.erase(args.begin() + i);
	}

}

bool Simple::ProcessMidasEvent(std::ostream& dest,
		TDataContainer& dataContainer,
		util::caen::DigitizerInfoRawData const& info) {

	using util::TWaveFormRawData;
	using util::TriggerInfoRawData;
	using util::TDcOffsetRawData;

	if (0 == (info.info().pattern.bits.channelTrigger & triggerMask)) {
		return true;
	}

	auto const& midasData = dataContainer.GetMidasData();
	auto const dcInfo = dataContainer.GetEventData < TDcOffsetRawData
			> (TDcOffsetRawData::BANK_NAME);
	auto const trgInfo = dataContainer.GetEventData < TriggerInfoRawData
			> (TriggerInfoRawData::bankName());

	std::size_t recordLength = 0;

	for (uint8_t ch = 0; ch < 8; ch++) {
		if (info.channelIncluded(ch)) {
			auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
					> (TWaveFormRawData::bankName(ch));
			if (wfRaw) {
				recordLength = std::max(recordLength, wfRaw->numOfSamples());
			}
		}
	}

	dest << "#EventInfo\n" << "TimeStamp\n" << midasData.GetTimeStamp() << "\n"
			<< "Run\t" << GetRun() << "\n" << "Device\t" << info.GetName()
			<< "\n" << "BoardId\t" << info.info().boardId << "\n"
			<< "ChannelMask\t" << info.info().channelMask << "\n"
			<< "EventCounter\t" << info.info().eventCounter << "\n"
			<< "FrontendIndex\t" << info.frontendIndex() << "\n"
			<< "DeviceTimeStamp\t" << info.timeStamp() << "\n"
			<< "DeviceTimeStampModule\t" << info.timeStampModule() << "\n"
			<< "TicksPerSample\t" << info.ticksPerSample() << "\n"
			<< "SamplesPerNuSecond\t" << info.samplesPerNuSecond() << "\n"
			<< "WaveformLength\t" << recordLength << "\n"
			<< "PreTriggerLength\t" << info.info().preTriggerLength << "\n"
			<< "BitsPerSample\t" << static_cast<int>(info.sampleWidthInBits())
			<< "\n" << "DcMultiplier\t" << info.dcMultiplier() << "\n\n";

	dest << "#Waveforms\n";

	{
		bool first = true;

		for (uint8_t ch = 0; ch < 8; ch++) {
			if (info.channelIncluded(ch)) {
				if (first) {
					first = false;
				} else {
					dest << '\t';
				}
				dest << "CH" << static_cast<int>(ch);
			}
		}

		dest << "\n";
	}

	for (std::size_t i = 0; i < recordLength; i++) {
		bool first = true;

		for (uint8_t ch = 0; ch < 8; ch++) {
			if (info.channelIncluded(ch)) {
				if (first) {
					first = false;
				} else {
					dest << '\t';
				}

				auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
						> (TWaveFormRawData::bankName(ch));
				if (wfRaw) {
					if (i < wfRaw->numOfSamples()) {
						dest << *(wfRaw->begin() + i);
					}
				}
			}

		}

		dest << "\n";
	}
	dest << "\n";

	if (dcInfo) {
		dest << "#DcOffsets\n";

		{
			bool first = true;

			for (uint8_t ch = 0; ch < 8; ch++) {
				if (first) {
					first = false;
				} else {
					dest << '\t';
				}
				dest << "CH" << static_cast<int>(ch);
			}

			dest << "\n";
		}

		dest << std::fixed << std::setprecision(6);

		for (std::size_t i = 0; i < 2; i++) {
			bool first = true;

			for (uint8_t ch = 0; ch < 8; ch++) {
				if (first) {
					first = false;
				} else {
					dest << '\t';
				}

				switch (i) {
				case 0:
					dest << dcInfo->dcOffset(ch);
					break;
				case 1:
					dest << info.dcBaseline(dcInfo->dcOffset(ch));
					break;
				}
			}

			dest << "\n";
		}
		dest << "\n";
	}

	if (trgInfo) {
		dest << "#Triggers\n";

		{
			bool first = true;

			for (uint8_t ch = 0; ch < 8; ch++) {
				if (first) {
					first = false;
				} else {
					dest << '\t';
				}
				dest << "CH" << static_cast<int>(ch);
			}

			dest << "\n";
		}

		for (std::size_t i = 0; i < 4; i++) {
			bool first = true;

			for (TriggerInfoRawData::channelno_type trgCh = 0; trgCh < 8;
					trgCh++) {

				if (first) {
					first = false;
				} else {
					dest << '\t';
				}

				auto const trgChInf = trgInfo->channelInfo(trgCh);

				switch (i) {
				case 0:
					if (trgChInf) {
						dest << (info.selfTrigger(trgCh) ? 1 : 0);
					} else {
						dest << 0;
					}
					break;

				case 1:
					if (trgChInf) {
						dest << (trgInfo->rising(*trgChInf) ? 1 : 0);
					} else {
						dest << 0;
					}
					break;

				case 2:
					if (trgChInf) {
						dest << trgInfo->threshold(*trgChInf);
					} else {
						dest << -1;
					}
					break;

				case 3: {
					auto const wfRaw = dataContainer.GetEventData
							< TWaveFormRawData
							> (TWaveFormRawData::bankName(trgCh));

					if (trgChInf && info.selfTrigger(trgCh) && wfRaw) {
						dest
								<< math::FindEdgeDistance(
										TriggerInfoRawData::rising(*trgChInf),
										TriggerInfoRawData::threshold(
												*trgChInf), wfRaw->begin(),
										wfRaw->end());
					} else {
						dest << -1;
					}
				}
					break;
				}
			}

			dest << "\n";
		}

		dest << "\n";
	}

	dest << "#EndOfEvent\n" << std::endl;

	return true;

}

}

}
