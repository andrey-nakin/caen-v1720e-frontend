#include <caen/error-holder.hxx>

namespace caen {

ErrorHolder::ErrorHolder() : errorCode(0) {
}

ErrorHolder::operator bool() const {

	return 0 == errorCode;

}

}

