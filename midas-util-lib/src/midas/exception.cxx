#include <sstream>
#include "midas/exception.hxx"

namespace midas {

static std::string programName = __FILE__;

Exception::Exception(INT const aStatus, std::string const& aMsg) :
		status(aStatus), msg(makeMsg(aStatus, aMsg)) {

	cm_msg(MERROR, programName.c_str(), msg.c_str(), status);

}

Exception::~Exception() {

}

const char* Exception::what() const throw () {

	return msg.c_str();

}

std::string Exception::makeMsg(INT const status, std::string const& msg) {

	std::stringstream s;
	s << msg;
	s << ", status " << status;
	return s.str();

}

void Exception::setProgramName(const char * const name) {

	programName = name;

}

}
