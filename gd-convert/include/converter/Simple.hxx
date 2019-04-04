#pragma once

#include <string>
#include <util/caen/DigitizerInfoRawData.hxx>
#include <Converter.hxx>

namespace gdc {

namespace converter {

class Simple: public Converter {
public:

	Simple();

	static std::string Name();

	void BeginRun(int transition, int run, int time) override;
	void ProcessMidasEvent(TDataContainer& dataContainer) override;

private:

	int currentRun;

	void ProcessMidasEvent(TDataContainer& dataContainer,
			util::caen::DigitizerInfoRawData const& info);
	std::string ConstructName(util::caen::DigitizerInfoRawData const& info);

};

}

}
