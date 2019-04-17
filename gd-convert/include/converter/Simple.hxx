#pragma once

#include <string>
#include <util/caen/DigitizerInfoRawData.hxx>
#include "FilePerRun.hxx"

namespace gdc {

namespace converter {

class Simple: public FilePerRun {

	static constexpr int VERSION = 1;

public:

	Simple();

	static std::string Name();

	bool ProcessMidasEvent(std::ostream& dest, TDataContainer& dataContainer)
			override;

	void Configure(std::vector<char*>& args) override;

	std::string FileExtension() const override;

private:

	uint16_t triggerMask;

	bool ProcessMidasEvent(std::ostream& dest, TDataContainer& dataContainer,
			util::caen::DigitizerInfoRawData const& info);

	void onNewFile(std::ostream& dest, std::string const& fileName) override;

};

}

}
