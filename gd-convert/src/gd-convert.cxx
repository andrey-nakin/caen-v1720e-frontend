#include <iostream>
#include <memory>
#include <vector>
#include <cstring>
#include <TDataContainer.hxx>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <TRootanaEventLoop.hxx>
#include <analyzer/exception/CommandLineException.hxx>

#pragma GCC diagnostic pop

namespace gdc {

namespace cmd {

constexpr char format[] = "-f";

}

namespace format {

constexpr char binary[] = "binary";

}

class Main: public TRootanaEventLoop {

	typedef TRootanaEventLoop Base;

	enum class Format {
		Binary
	};

public:

	Main() :
			format(Format::Binary) {
		SetOnlineName("gd-convert");
		UseBatchMode();
	}

	void BeginRun(int const transition, int const run, int const time)
			override {

	}

	bool ProcessMidasEvent(TDataContainer & dataContainer) {

//		auto const v1720Info = dataContainer.GetEventData < V1720InfoRawData
//				> (V1720InfoRawData::bankName());
//		if (v1720Info) {
//			for (uint8_t channelNo = 0;
//					channelNo < caen::v1720::NUM_OF_CHANNELS; channelNo++) {
//				if (v1720Info->channelIncluded(channelNo)) {
//					auto const wfRaw = dataContainer.GetEventData
//							< TWaveFormRawData
//							> (TWaveFormRawData::bankName(channelNo));
//					if (wfRaw) {
//						auto const numOfSamples = wfRaw->numOfSamples();
//						if (numOfSamples > 0) {
//
//						}
//					}
//				}
//			}
//		}

		return true;

	}

	void EndRun(int const transition, int const run, int const time) override {

	}

	int ExecuteLoopOverriden(int const argc, char* argv[]) {
		try {
			std::vector<char*> arguments = AnalyzeCommandLine(argc, argv);
			return Base::ExecuteLoop(arguments.size(), &arguments[0]);
		} catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
			return -1;
		}
	}

private:

	Format format;

	static bool StartsWith(const char* const s, const char* const substr) {
		return s == std::strstr(s, substr);
	}

	static Format ParseFormat(const char* const s) {
		if (s && *s) {
			if (0 == std::strcmp(s, format::binary)) {
				return Format::Binary;
			} else {
				// unknown format
				throw analyzer::exception::CommandLineException()
						<< "Bad format: " << s;
			}
		}

		return Format::Binary;
	}

	std::vector<char*> AnalyzeCommandLine(int const argc, char* argv[]) {
		std::vector<char*> arguments;
		arguments.push_back(argv[0]);	//	executable name

		for (int i = 1; i < argc; i++) {
			if (StartsWith(argv[i], cmd::format)) {
				format = ParseFormat(argv[i] + std::strlen(cmd::format));
			} else {
				arguments.push_back(argv[i]);	//	standard parameter
			}
		}

		return arguments;
	}

};

}
int main(int argc, char* argv[]) {

	using namespace gdc;

	Main::CreateSingleton<Main>();
	return static_cast<Main&>(Main::Get()).ExecuteLoopOverriden(argc, argv);

}
