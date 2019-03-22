#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <math/StatAccum.hxx>

namespace math {

TEST(StatAccum, Generic) {

	std::array<int8_t, 7> const src = { 0, 1, 3, 6, 3, -2, 0 };
	auto sa = math::MakeStatAccum<int16_t>(std::begin(src), std::end(src));

	EXPECT_EQ(7, sa.GetCount());
	EXPECT_EQ(-2, sa.GetMinValue());
	EXPECT_EQ(6, sa.GetMaxValue());
	EXPECT_EQ(2, sa.GetRoughMean());
	EXPECT_NEAR(1.571429, sa.GetMean(), 0.5e-6);
	EXPECT_EQ(7, sa.GetRoughVariance());
	EXPECT_NEAR(6.952381, sa.GetVariance(), 0.5e-6);
	EXPECT_NEAR(2.636737, sa.GetStd(), 0.5e-6);
	EXPECT_EQ(26, sa.GetStdScaled(10.0));
	EXPECT_EQ(264, sa.GetStdScaled < int16_t > (100.0));

}

TEST(StatAccum, NoType) {

	std::array<int8_t, 7> const src = { 0, 1, 3, 6, 3, -2, 0 };
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

TEST(StatAccum, EmptyRange) {

	std::array<int8_t, 7> const src = { 0, 1, 3, 6, 3, -2, 0 };
	auto sa = math::MakeStatAccum(src.end(), src.end());

	EXPECT_EQ(0, sa.GetCount());
	EXPECT_EQ(0, sa.GetRoughMean());
	EXPECT_EQ(0.0, sa.GetMean());
	EXPECT_EQ(0, sa.GetRoughVariance());
	EXPECT_EQ(0.0, sa.GetVariance());

}

TEST(StatAccum, InvalidRange) {

	std::array<int8_t, 7> const src = { 0, 1, 3, 6, 3, -2, 0 };
	auto sa = math::MakeStatAccum(src.end() + 1, src.end());

	EXPECT_EQ(0, sa.GetCount());

}

TEST(StatAccum, AppendGeneric) {

	std::array<int8_t, 7> const src1 = { 1, 2, 3, 4, 5, 6, 7 };
	std::array<int8_t, 5> const src2 = { 1, 2, 3, 4, 5 };
	auto sa = math::MakeStatAccum(src2.begin(), src2.end(),
			math::MakeStatAccum(src1.begin(), src1.end()));

	EXPECT_EQ(12, sa.GetCount());
	EXPECT_EQ(1, sa.GetMinValue());
	EXPECT_EQ(7, sa.GetMaxValue());
	EXPECT_EQ(4, sa.GetRoughMean());
	EXPECT_NEAR(3.583333, sa.GetMean(), 0.5e-6);
	EXPECT_EQ(4, sa.GetRoughVariance());
	EXPECT_NEAR(3.719697, sa.GetVariance(), 0.5e-6);
	EXPECT_NEAR(1.928652, sa.GetStd(), 0.5e-6);
	EXPECT_EQ(19, sa.GetStdScaled(10.0));

}

TEST(StatAccum, AppendCommutativity) {

	std::array<int8_t, 7> const src1 = { 1, 2, 3, 4, 5, 6, 7 };
	std::array<int8_t, 5> const src2 = { 1, 2, 3, 4, 5 };

	EXPECT_TRUE(
			math::MakeStatAccum(src2.begin(), src2.end(),
					math::MakeStatAccum(src1.begin(), src1.end()))
					== math::MakeStatAccum(src1.begin(), src1.end(),
							math::MakeStatAccum(src2.begin(), src2.end())));

}

TEST(StatAccum, AppendEmpty) {

	std::array<int8_t, 7> const src1 = { 1, 2, 3, 4, 5, 6, 7 };
	std::array<int8_t, 0> const src2 = { };

	auto const sa1 = math::MakeStatAccum(src1.begin(), src1.end());
	auto const sa2 = math::MakeStatAccum(src1.begin(), src1.end(),
			math::MakeStatAccum(src2.begin(), src2.end()));

	EXPECT_TRUE(sa1 == sa2);

	auto const sa3 = math::MakeStatAccum(src1.begin(), src1.end());
	auto const sa4 = math::MakeStatAccum(src2.begin(), src2.end(),
			math::MakeStatAccum(src1.begin(), src1.end()));

	EXPECT_TRUE(sa3 == sa4);

}

TEST(StatAccum, AppendBad) {

	std::array<int8_t, 7> const src1 = { 1, 2, 3, 4, 5, 6, 7 };
	std::array<int8_t, 3> const src2 = { 1, 2, 3 };

	auto const sa1 = math::MakeStatAccum(src1.begin(), src1.end());
	auto const sa2 = math::MakeStatAccum(src1.begin(), src1.end(),
			math::MakeStatAccum(src2.end() + 100, src2.end()));

	EXPECT_TRUE(sa1 == sa2);

	auto const sa3 = math::MakeStatAccum(src1.begin(), src1.end());
	auto const sa4 = math::MakeStatAccum(src2.end() + 100, src2.end(),
			math::MakeStatAccum(src1.begin(), src1.end()));

	EXPECT_TRUE(sa3 == sa4);

}

TEST(StatAccum, ExcludeGeneric) {

	std::array<int8_t, 10> const src = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	auto sa = math::MakeStatAccum(src.begin(), src.end(), src.begin() + 3,
			src.begin() + 5);

	EXPECT_EQ(8, sa.GetCount());
	EXPECT_EQ(0, sa.GetMinValue());
	EXPECT_EQ(9, sa.GetMaxValue());
	EXPECT_NEAR(4.75, sa.GetMean(), 0.5e-6);
	EXPECT_NEAR(11.357143, sa.GetVariance(), 0.5e-6);

}

TEST(StatAccum, ExcludeBeginning) {

	std::array<int8_t, 10> const src = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	auto sa = math::MakeStatAccum(src.begin(), src.end(), src.begin(),
			src.begin() + 5);

	EXPECT_EQ(5, sa.GetCount());
	EXPECT_EQ(5, sa.GetMinValue());
	EXPECT_EQ(9, sa.GetMaxValue());
	EXPECT_NEAR(7, sa.GetMean(), 0.5e-6);
	EXPECT_NEAR(2.5, sa.GetVariance(), 0.5e-6);

}

TEST(StatAccum, ExcludeBeyondEnd) {

	std::array<int8_t, 10> const src = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	auto sa = math::MakeStatAccum(src.begin(), src.end(), src.begin() + 3,
			src.begin() + 100);

	EXPECT_EQ(3, sa.GetCount());
	EXPECT_EQ(0, sa.GetMinValue());
	EXPECT_EQ(2, sa.GetMaxValue());
	EXPECT_NEAR(1, sa.GetMean(), 0.5e-6);
	EXPECT_NEAR(1, sa.GetVariance(), 0.5e-6);

}

TEST(StatAccum, ExcludeEmpty) {

	std::array<int8_t, 10> const src = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	auto sa = math::MakeStatAccum(src.begin(), src.end(), src.begin() + 3,
			src.begin() + 3);

	EXPECT_EQ(10, sa.GetCount());
	EXPECT_EQ(0, sa.GetMinValue());
	EXPECT_EQ(9, sa.GetMaxValue());
	EXPECT_NEAR(4.5, sa.GetMean(), 0.5e-6);
	EXPECT_NEAR(9.166667, sa.GetVariance(), 0.5e-6);

}

TEST(StatAccum, ExcludeAll) {

	std::array<int8_t, 10> const src = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	auto sa = math::MakeStatAccum(src.begin(), src.end(), src.begin(),
			src.end());

	EXPECT_EQ(0, sa.GetCount());
	EXPECT_EQ(0, sa.GetRoughMean());

}

}
