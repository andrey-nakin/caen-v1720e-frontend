#ifndef	__CAEN_ERROR_HOLDER_HXX__
#define	__CAEN_ERROR_HOLDER_HXX__

#include <string>
#include <CAENDigitizer.h>
#include "exception.hxx"

namespace caen {

class ErrorHolder {
protected:

	ErrorHolder();

public:

	template<class Function>
	void command(std::string const& msg, Function action) {
		auto const status = action();
		if (CAEN_DGTZ_Success != status) {
			throw Exception(status, msg);
		}
	}

	template<class MessageBuilder, class Function>
	void commandE(MessageBuilder msgBuilder, Function action) {
		auto const status = action();
		if (CAEN_DGTZ_Success != status) {
			throw Exception(status, msgBuilder());
		}
	}

	static const char* errorMessage(CAEN_DGTZ_ErrorCode ec);

};

}

#endif	//	__CAEN_ERROR_HOLDER_HXX__

