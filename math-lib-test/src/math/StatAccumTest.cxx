#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <math/StatAccum.hxx>

namespace math {

TEST(StatAccum, Generic) {

	std::array<uint8_t, 7> const src = {0, 1, 3, 6, 3, 1, 0};
	auto sa = math::MakeStatAccum<int16_t>(std::begin(src), std::end(src));

	EXPECT_EQ(7, sa.GetCount());

}

}
