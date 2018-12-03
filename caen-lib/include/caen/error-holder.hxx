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

};

}

#endif	//	__CAEN_ERROR_HOLDER_HXX__

