#pragma once

#include <string>
#include <util/caen/DigitizerInfoRawData.hxx>
#include "FilePerRun.hxx"

namespace gdc {

namespace converter {

class Simple: public FilePerRun {
public:

	Simple();

	static std::string Name();

	void BeginRun(int transition, int run, int time) override;
	bool ProcessMidasEvent(std::ostream& dest, TDataContainer& dataContainer)
			override;
	void Configure(std::vector<char*>& args) override;
	std::string FileExtension() const override {

		return ".txt";

	}

private:

	int currentRun;
	uint16_t triggerMask;
	unsigned eventCounter, maxEvents;

	bool ProcessMidasEvent(std::ostream& dest, TDataContainer& dataContainer,
			util::caen::DigitizerInfoRawData const& info);
	std::string ConstructName(util::caen::DigitizerInfoRawData const& info);

};

}

}
