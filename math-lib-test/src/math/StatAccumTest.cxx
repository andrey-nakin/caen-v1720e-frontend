#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <math/StatAccum.hxx>

namespace math {

TEST(StatAccum, Generic) {

	std::array<int8_t, 7> const src = {0, 1, 3, 6, 3, -2, 0};
	auto sa = math::MakeStatAccum<int16_t>(std::begin(src), std::end(src));

	EXPECT_EQ(7, sa.GetCount());
	EXPECT_EQ(-2, sa.GetMinValue());
	EXPECT_EQ(6, sa.GetMaxValue());
	EXPECT_EQ(2, sa.GetRoughMean());
	EXPECT_NEAR(1.571429, sa.GetMean(), 0.5e-6);
	EXPECT_EQ(7, sa.GetRoughVariance());
	EXPECT_NEAR(6.952381, sa.GetVariance(), 0.5e-6);
	EXPECT_NEAR(2.636737, sa.GetStd(), 0.5e-6);

}

TEST(StatAccum, NoType) {

	std::array<int8_t, 7> const src = {0, 1, 3, 6, 3, -2, 0};
	auto sa = math::MakeStatAccum(std::begin(src), std::end(src));

	EXPECT_EQ(7, sa.GetCount());
	EXPECT_EQ(-2, sa.GetMinValue());
	EXPECT_EQ(6, sa.GetMaxValue());
	EXPECT_EQ(2, sa.GetRoughMean());
	EXPECT_NEAR(1.571429, sa.GetMean(), 0.5e-6);
	EXPECT_EQ(7, sa.GetRoughVariance());
	EXPECT_NEAR(6.952381, sa.GetVariance(), 0.5e-6);
	EXPECT_NEAR(2.636737, sa.GetStd(), 0.5e-6);

}

}
