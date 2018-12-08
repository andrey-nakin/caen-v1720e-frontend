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
public:

	typedef int handle_type;

	Handle(int linkNum, int conetNode, uint32_t vmeBaseAddr);
	Handle(Handle&& h);
	~Handle();

	operator handle_type() {

		return handle;

	}

	uint32_t readRegister(uint32_t reg);
	void writeRegister(uint32_t reg, uint32_t regData);

	template<class Function>
	void hCommand(std::string const& msg, Function action) {
		ErrorHolder::command(msg, [this, action]() {return action(handle);});
	}

	void hCommand(std::string const& msg, CAEN_DGTZ_ErrorCode (*action)(handle_type)) {
		ErrorHolder::command(msg, [this, action]() {return action(handle);});
	}

	template<class MessageBuilder, class Function>
	void hCommandE(MessageBuilder msgBuilder, Function action) {
		ErrorHolder::commandE(msgBuilder,
				[this, action]() {return action(handle);});
	}

private:

	handle_type handle;

};

}

#endif	//	__CAEN_HANDLE_HXX__

