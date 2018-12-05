#include "frontend/locker.hxx"

namespace fe {

Locker::Locker(std::atomic_bool& v) : value(v) {

	value.store(true, std::memory_order_release);

}

Locker::~Locker() {

	value.store(false, std::memory_order_release);

}

}
