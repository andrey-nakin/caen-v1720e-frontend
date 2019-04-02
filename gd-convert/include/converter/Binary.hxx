#pragma once

#include <string>
#include <Converter.hxx>

namespace gdc {

namespace converter {

class Binary: public Converter {
public:

	using Converter::Converter;

	static std::string Name();

	void ProcessMidasEvent(std::ostream& dest, TDataContainer & dataContainer)
			override;

	std::string FileExtension() const override;
	std::ios_base::openmode FileMode() const override;

};

}

}
