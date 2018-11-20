#ifndef	__CAEN_HANDLE_HXX__
#define	__CAEN_HANDLE_HXX__

#include <cstdint>
#include "noncopyable.hxx"
#include "error-holder.hxx"

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

};

}

#endif	//	__CAEN_HANDLE_HXX__

