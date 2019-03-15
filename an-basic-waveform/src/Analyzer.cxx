#include <iostream>
#include <memory>
#include <TDataContainer.hxx>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <TRootanaEventLoop.hxx>

#pragma GCC diagnostic pop

#include <analyzer/util/AnalyzerUtils.hxx>
#include "TAnaManager.hxx"

namespace bwf {

constexpr char NAME[] = "an-basic-waveform";

class Analyzer: public TRootanaEventLoop {

public:
	std::unique_ptr<TAnaManager> anaManager;

	Analyzer() {
		SetOnlineName(NAME);
		// DisableAutoMainWindow();
		UseBatchMode();
	}

	void Initialize() {

#ifdef HAVE_THTTP_SERVER
		std::cout << "Using THttpServer in read/write mode" << std::endl;

		SetTHttpServerReadWrite();
		GetTHttpServer()->SetCors("*");
#endif

	}

	void InitManager() {

		anaManager.reset(
				new TAnaManager(GetODB(),
						analyzer::util::AnalyzerUtils::OdbRootKey(NAME)));

	}

	void BeginRun(int const transition, int const run, int const time)
			override {

		InitManager();
		anaManager->BeginRun(transition, run, time);

	}

	bool ProcessMidasEvent(TDataContainer & dataContainer) {

		if (!anaManager) {
			InitManager();
		}

		anaManager->ProcessMidasEvent(dataContainer);

		return true;

	}

	void EndRun(int const transition, int const run, int const time) override {

		anaManager->EndRun(transition, run, time);

	}

};

}
int main(int argc, char * argv[]) {

	bwf::Analyzer::CreateSingleton<bwf::Analyzer>();
	return bwf::Analyzer::Get().ExecuteLoop(argc, argv);

}
