#include <sstream>
#include "caen/exception.hxx"

namespace caen {

Exception::Exception(CAEN_DGTZ_ErrorCode const anErrorCode, std::string const& aMsg)
: errorCode(anErrorCode), msg(makeMsg(anErrorCode, aMsg)) {

}

Exception::~Exception() {

}

std::string Exception::makeMsg(CAEN_DGTZ_ErrorCode const errorCode, std::string const& msg) {

	std::stringstream s;
	s << "CAEN Error " << errorCode << " when " << msg;
	return s.str();

}

}
