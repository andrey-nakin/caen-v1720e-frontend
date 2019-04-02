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

	virtual void ConstructFileName(std::ostream& dest, int run) const;
	virtual std::string FileExtension() const = 0;
	virtual std::ios_base::openmode FileMode() const;

};

}
