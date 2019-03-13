#pragma once

#include <limits>

namespace math {

template<typename IntType, uint8_t BitNum>
class IntOp {
public:

	typedef IntType value_type;

	static constexpr IntType module() {

		return static_cast<IntType>(0x1) << BitNum;

	}

	static constexpr IntType mask() {

		return ~module();

	}

	static IntType value(IntType const v) {

		return v & mask();

	}

	static IntType sub(IntType const a, IntType const b) {

		return (value(a) - value(b)) % module();

	}

	static_assert(std::numeric_limits<IntType>::min() == 0, "Signed types are not supported");
	static_assert(std::numeric_limits<IntType>::max() > module(), "BitNum is too big");
	static_assert(module() > 0, "BitNum is too big");

};

}
