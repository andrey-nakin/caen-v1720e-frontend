#pragma once

#include <iostream>
#include <vector>
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
	virtual bool ProcessMidasEvent(TDataContainer& dataContainer) = 0;

	virtual void Configure(std::vector<char*>& args);

protected:

	static bool StartsWith(const char* const s, const char* const substr);

};

}
