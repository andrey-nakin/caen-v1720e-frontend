#ifndef	__midas_util_lib_exception_hxx__
#define	__midas_util_lib_exception_hxx__

#include <exception>
#include <string>
#include <midas.h>

namespace midas {

class Exception: public std::exception {

	INT const status;
	std::string const msg;

	static std::string makeMsg(INT status, std::string const& msg);

public:

	Exception(INT status, std::string const& msg);
	~Exception() override;

	INT getStatus() const {

		return status;

	}

	const char* what() const throw () override;

	static void setProgramName(const char *name);

};

}

#endif	//	__midas_util_lib_exception_hxx__
