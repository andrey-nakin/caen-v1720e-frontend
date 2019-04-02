#include <iostream>
#include <memory>
#include <vector>
#include <cstring>
#include <sstream>
#include <fstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <TRootanaEventLoop.hxx>
#include <analyzer/exception/CommandLineException.hxx>
#include <converter/Binary.hxx>

#pragma GCC diagnostic pop

namespace gdc {

namespace cmd {

constexpr char format[] = "-f";

}

class Main: public TRootanaEventLoop {
public:

	Main() {
		SetOnlineName("gd-convert");
		UseBatchMode();
	}

	void BeginRun(int /* transition */, int const run, int /* time */)
			override {

		std::stringstream s;
		converter->ConstructFileName(s, run);

		std::string filename = s.str();
		dest = std::unique_ptr < std::ostream
				> (new std::ofstream(filename, converter->FileMode()));

	}

	bool ProcessMidasEvent(TDataContainer & dataContainer) {

		converter->ProcessMidasEvent(*dest, dataContainer);
		return true;

	}

	void EndRun(int /* transition */, int /* run */, int /* time */) override {

		if (dest) {
			dest = nullptr;
		}

	}

	int ExecuteLoopOverriden(int const argc, char* argv[]) {

		try {
			std::vector<char*> arguments = AnalyzeCommandLine(argc, argv);
			return ExecuteLoop(arguments.size(), &arguments[0]);
		} catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
			return -1;
		}

	}

private:

	std::unique_ptr<Converter> converter;
	std::unique_ptr<std::ostream> dest;

	static bool StartsWith(const char* const s, const char* const substr) {

		return s == std::strstr(s, substr);

	}

	static std::unique_ptr<Converter> ParseFormat(const char* const s) {

		if (s && *s) {
			if (converter::Binary::Name() == s) {
				return std::unique_ptr < Converter > (new converter::Binary);
			} else {
				// unknown format
				throw analyzer::exception::CommandLineException()
						<< "Bad format: " << s;
			}
		}

		return DefaultConverter();

	}

	static std::unique_ptr<Converter> DefaultConverter() {

		return std::unique_ptr < Converter > (new converter::Binary);

	}

	std::vector<char*> AnalyzeCommandLine(int const argc, char* argv[]) {

		std::vector<char*> arguments;
		arguments.push_back(argv[0]);	//	executable name

		for (int i = 1; i < argc; i++) {
			if (StartsWith(argv[i], cmd::format)) {
				converter = std::move(
						ParseFormat(argv[i] + std::strlen(cmd::format)));
			} else {
				arguments.push_back(argv[i]);	//	standard parameter
			}
		}

		if (!converter) {
			converter = std::move(DefaultConverter());
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
