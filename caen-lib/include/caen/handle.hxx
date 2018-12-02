#ifndef	__CAEN_HANDLE_HXX__
#define	__CAEN_HANDLE_HXX__

#include <cstdint>
#include <functional>
#include <string>
#include <CAENDigitizer.h>
#include "noncopyable.hxx"
#include "error-holder.hxx"
#include "exception.hxx"

namespace caen {

class Handle final : public Noncopyable, public ErrorHolder {

	int handle;

public:

	Handle(int linkNum, int conetNode, ::uint32_t vmeBaseAddr);
	Handle(Handle&& h);
	~Handle();

	operator int() {

		return handle;

	}

	template<class Function>
	void command(std::string const& msg, Function action) {
		auto const res = action(handle);
		if (CAEN_DGTZ_Success != res) {
			throw Exception(res, msg);
		}
	}

};

}

#endif	//	__CAEN_HANDLE_HXX__

