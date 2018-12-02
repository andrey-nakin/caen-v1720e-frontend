#ifndef	__CAEN_EXCEPTION_HXX__
#define	__CAEN_EXCEPTION_HXX__

#include <exception>
#include <string>
#include <CAENDigitizer.h>

namespace caen {

class Exception : public std::exception {

	CAEN_DGTZ_ErrorCode const errorCode;
	std::string const msg;

	static std::string makeMsg(CAEN_DGTZ_ErrorCode errorCode, std::string const& msg);

public:

	Exception(CAEN_DGTZ_ErrorCode errorCode, std::string const& msg);
	~Exception() override;

	CAEN_DGTZ_ErrorCode getErrorCode() const {

		return errorCode;

	}

    const char* what() const throw() override {

    	return msg.c_str();

    }

};

}

#endif	//	__CAEN_EXCEPTION_HXX__

