#include <caen/error-holder.hxx>

namespace caen {

ErrorHolder::ErrorHolder() : errorCode(CAEN_DGTZ_Success) {
}

ErrorHolder::ErrorHolder(CAEN_DGTZ_ErrorCode const anErrorCode) : errorCode(anErrorCode) {

};

ErrorHolder::operator bool() const {

	return CAEN_DGTZ_Success == errorCode;

}

}

