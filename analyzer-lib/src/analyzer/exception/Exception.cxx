#include <analyzer/exception/Exception.hxx>

namespace analyzer {

namespace exception {

Exception::Exception() {
}

Exception::Exception(Exception const& e) {
	msgStream << e.msgStream.str();
}

Exception::~Exception() {

}

}

}
