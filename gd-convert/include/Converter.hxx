#pragma once

#include <iostream>
#include <TDataContainer.hxx>

namespace gdc {

class Converter {
public:

	Converter();
	Converter(Converter const&) = delete;
	virtual ~Converter();

	virtual void ProcessMidasEvent(std::ostream& dest,
			TDataContainer& dataContainer) = 0;

};

}
