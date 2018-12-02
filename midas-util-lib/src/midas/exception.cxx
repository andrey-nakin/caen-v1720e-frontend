#include <sstream>
#include "midas/exception.hxx"

#ifndef NEED_NO_EXTERN_C
extern "C" {
#endif

extern const char *frontend_name;

#ifndef NEED_NO_EXTERN_C
}
#endif

namespace midas {

Exception::Exception(INT const aStatus, std::string const& aMsg)
	: status(aStatus), msg(makeMsg(aStatus, aMsg)) {

	cm_msg(MERROR, frontend_name, msg.c_str(), status);

}

Exception::~Exception() {

}

const char* Exception::what() const throw() {

	return msg.c_str();

}

std::string Exception::makeMsg(INT const status, std::string const& msg) {

	std::stringstream s;
	s << msg;
	s << ", status " << status;
	return s.str();

}

}
