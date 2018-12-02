#ifndef	__CAEN_ERROR_HOLDER_HXX__
#define	__CAEN_ERROR_HOLDER_HXX__

#include <CAENDigitizer.h>

namespace caen {

class ErrorHolder {
protected:

	CAEN_DGTZ_ErrorCode errorCode;

public:

	ErrorHolder();

	operator bool() const;

	CAEN_DGTZ_ErrorCode getErrorCode() const {

		return errorCode;

	}

};

}

#endif	//	__CAEN_ERROR_HOLDER_HXX__

