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

	uint32_t readRegister(uint32_t reg);
	void writeRegister(uint32_t reg, uint32_t regData);

	template<class Function>
	void command(std::string const& msg, Function action) {
		ErrorHolder::command(
				msg,
				[this, action]() {return action(handle);}
		);
	}

};

}

#endif	//	__CAEN_HANDLE_HXX__

