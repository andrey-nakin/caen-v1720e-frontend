#include <iostream>
#include <memory>
#include <TDataContainer.hxx>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <TRootanaEventLoop.hxx>

#pragma GCC diagnostic pop

namespace gdc {

class Main: public TRootanaEventLoop {
public:

	Main() {
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

};

}
int main(int argc, char* argv[]) {

	using namespace gdc;

	Main::CreateSingleton<Main>();
	return Main::Get().ExecuteLoop(argc, argv);

}
