#include <sstream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <util/caen/V1720InfoRawData.hxx>
#include <util/caen/V1724InfoRawData.hxx>
#include <util/TWaveFormRawData.hxx>
#include <util/TriggerInfoRawData.hxx>
#include <converter/Simple.hxx>

namespace gdc {

namespace converter {

namespace cmd {

constexpr char triggerMask[] = "-tm";
constexpr char events[] = "-e";

}

static bool StartsWith(const char* const s, const char* const substr) {

	return s == std::strstr(s, substr);

}

Simple::Simple() :
		currentRun(0), triggerMask(0xffff), eventCounter(0), maxEvents(0) {

}

std::string Simple::Name() {

	return "simple";

}

void Simple::BeginRun(int const transition, int const run, int time) {

	FilePerRun::BeginRun(transition, run, time);
	currentRun = run;

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
		} else if (StartsWith(args[i], cmd::events)) {
			maxEvents = std::stoi(args[i] + std::strlen(cmd::events));
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

	if (maxEvents > 0 && maxEvents <= eventCounter) {
		return false;
	}

	if (0 == (info.info().pattern.bits.channelTrigger & triggerMask)) {
		return true;
	}

	//std::ofstream dest(ConstructName(info));
//	auto& dest = std::cout;

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

	dest << "EventInfo\n" << "Device\t" << info.GetName() << "\n" << "BoardId\t"
			<< info.info().boardId << "\n" << "ChannelMask\t"
			<< info.info().channelMask << "\n" << "EventCounter\t"
			<< info.info().eventCounter << "\n" << "FrontendIndex\t"
			<< info.frontendIndex() << "\n" << "TimeStamp\t"
			<< info.timeStampDifferenceInSamples(0) << "\n" << "TimeStampNs\t"
			<< info.timeStampDifferenceInNs(0) << "\n" << "PreTriggerLength\t"
			<< info.info().preTriggerLength << "\n" << "RecordLength\t"
			<< recordLength << "\n" << "BitsInSample\t"
			<< static_cast<int>(info.sampleWidthInBits()) << "\n\n";

//	if (trgInfo) {
//		for (TriggerInfoRawData::channelno_type trgCh = 0; trgCh < 8; trgCh++) {
//			auto const trgChInf = trgInfo->channelInfo(trgCh);
//			if (trgChInf) {
//				dest << "\tTRG" << static_cast<int>(trgCh);
//			}
//		}
//	}
//
//	dest << "\n";
//
//	for (std::size_t i = 0; i < recordLength; i++) {
//
//		switch (i) {
//		case 0:
//			dest << "Type\t" << info.GetName();
//			break;
//
//		case 1:
//			dest << "BoardId\t" << info.info().boardId;
//			break;
//
//		case 2:
//			dest << "ChannelMask\t" << info.info().channelMask;
//			break;
//
//		case 3:
//			dest << "EventCounter\t" << info.info().eventCounter;
//			break;
//
//		case 4:
//			dest << "FrontendIndex\t" << info.frontendIndex();
//			break;
//
//		case 5:
//			dest << "TimeStamp\t" << info.timeStampDifferenceInSamples(0);
//			break;
//
//		case 6:
//			dest << "TimeStampNs\t" << info.timeStampDifferenceInNs(0);
//			break;
//
//		case 7:
//			dest << "PreTriggerLength\t" << info.info().preTriggerLength;
//			break;
//
//		case 8:
//			dest << "RecordLength\t" << recordLength;
//			break;
//
//		case 9:
//			dest << "BitsInSample\t"
//					<< static_cast<int>(info.sampleWidthInBits());
//			break;
//
//		case 10:
//		case 11:
//		case 12:
//		case 13:
//		case 14:
//		case 15:
//		case 16:
//		case 17: {
//			auto const trgCh = i - 10;
//			dest << "TrgCh" << trgCh << "\t"
//					<< (info.selfTrigger(trgCh) ? "TRUE" : "FALSE");
//		}
//			break;
//
//		case 18:
//			dest << "TrgExt\t" << (info.extTrigger() ? "TRUE" : "FALSE");
//			break;
//
//		default:
//			dest << "\t-";
//		}
//
//		for (uint8_t ch = 0; ch < 8; ch++) {
//			dest << '\t';
//
//			if (info.channelIncluded(ch)) {
//				auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
//						> (TWaveFormRawData::bankName(ch));
//				if (wfRaw) {
//					if (i < wfRaw->numOfSamples()) {
//						dest << *(wfRaw->begin() + i);
//					}
//				}
//			}
//
//		}
//
//		if (trgInfo) {
//			for (TriggerInfoRawData::channelno_type trgCh = 0; trgCh < 8;
//					trgCh++) {
//				auto const trgChInf = trgInfo->channelInfo(trgCh);
//				if (trgChInf) {
//					dest << '\t';
//					switch (i) {
//					case 0:
//						dest << static_cast<int>(trgInfo->channel(*trgChInf));
//						break;
//					case 1:
//						dest << trgInfo->threshold(*trgChInf);
//						break;
//					case 2:
//						dest << (trgInfo->rising(*trgChInf) ? "TRUE" : "FALSE");
//						break;
//					}
//				}
//			}
//		}
//
//		dest << "\n";
//	}

	++eventCounter;
	dest << "." << std::endl;

	return true;

}

std::string Simple::ConstructName(
		util::caen::DigitizerInfoRawData const& info) {

	std::stringstream s;
	s << "run" << std::setfill('0') << std::setw(5) << currentRun << ".event"
			<< std::setw(9) << info.info().eventCounter << ".txt";
	return s.str();

}

}

}
