#include <cstring>
#include <Converter.hxx>

namespace gdc {

Converter::Converter() {
}

Converter::~Converter() {
}

void Converter::BeginRun(int /* transition */, int /* run */, int /* time */) {

	// null implementation

}

void Converter::EndRun(int /* transition */, int /* run */, int /* time */) {

	// null implementation

}

void Converter::Configure(std::vector<char*>& /* args */) {

	// null implementation

}

bool Converter::StartsWith(const char* const s, const char* const substr) {

	return s == std::strstr(s, substr);

}

}
