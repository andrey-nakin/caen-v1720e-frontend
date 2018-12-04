#include "frontend/locker.hxx"

namespace fe {

Locker::Locker(std::atomic_bool& v) : value(v) {

	value = true;

}

Locker::~Locker() {

	value = false;

}

}
