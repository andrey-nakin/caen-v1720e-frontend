#pragma once

#include <exception>
#include <sstream>
#include "Exception.hxx"

namespace analyzer {

namespace exception {

class CommandLineException: public Exception {
public:

	CommandLineException();
	using Exception::Exception;

};

}

}
