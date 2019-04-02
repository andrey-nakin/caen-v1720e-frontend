#include <iomanip>
#include <Converter.hxx>

namespace gdc {

Converter::Converter() {
}

Converter::~Converter() {
}

void Converter::ConstructFileName(std::ostream& dest, int const run) const {

	dest << "run" << std::setfill('0') << std::setw(5) << run
			<< FileExtension();

}

std::ios_base::openmode Converter::FileMode() const {

	return std::ios_base::out;

}

}
