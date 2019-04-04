#pragma once

#include <string>
#include <Converter.hxx>

namespace gdc {

namespace converter {

class Simple: public Converter {
public:

	Simple();

	static std::string Name();

	void ProcessMidasEvent(TDataContainer& dataContainer) override;

private:

};

}

}
