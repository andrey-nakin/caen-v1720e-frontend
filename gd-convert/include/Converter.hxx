#pragma once

#include <iostream>
#include <TDataContainer.hxx>

namespace gdc {

class Converter {

	Converter(Converter const&) = delete;
	Converter& operator=(Converter const&) = delete;

public:

	Converter();
	virtual ~Converter();

	virtual void BeginRun(int transition, int run, int time);
	virtual void EndRun(int transition, int run, int time);
	virtual void ProcessMidasEvent(TDataContainer& dataContainer) = 0;

};

}
