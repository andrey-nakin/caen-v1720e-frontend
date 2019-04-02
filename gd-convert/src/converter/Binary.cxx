#include <converter/Binary.hxx>

namespace gdc {

namespace converter {

std::string Binary::Name() {

	return "binary";

}

void Binary::ProcessMidasEvent(std::ostream& dest,
		TDataContainer & dataContainer) {

}

std::string Binary::FileExtension() const {

	return ".data";

}

std::ios_base::openmode Binary::FileMode() const {

	return std::ios_base::out | std::ios::binary;

}

}

}
