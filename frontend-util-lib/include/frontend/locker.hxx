#ifndef	__frontend_util_lib_locker_hxx__
#define	__frontend_util_lib_locker_hxx__

#include <atomic>

namespace fe {

class Locker {

	std::atomic_bool& value;

public:

	Locker(std::atomic_bool&);
	~Locker();

	Locker(const Locker&) = delete;
	Locker& operator=(const Locker&) = delete;

};

}

#endif	//	__frontend_util_lib_locker_hxx__
