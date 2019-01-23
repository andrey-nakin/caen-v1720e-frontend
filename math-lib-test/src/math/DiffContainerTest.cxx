#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <math/DiffContainer.hxx>

namespace math {

TEST(DiffContainer, Generic) {

	std::array<uint8_t, 7> const src = { 0, 1, 3, 6, 3, 1, 0 };
	auto const diff = 3;
	auto dc = math::MakeDiffContainer<int8_t>(std::begin(src), std::end(src),
			diff);

	auto a = std::begin(dc);
	auto const b = std::end(dc);

	EXPECT_EQ(4, std::distance(a, b));

	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
	EXPECT_EQ(6, *a);

	std::advance(a, 1);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
	EXPECT_EQ(2, *a);

	std::advance(a, 1);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
	EXPECT_EQ(-2, *a);

	std::advance(a, 1);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
	EXPECT_EQ(-6, *a);

	std::advance(a, 1);
	EXPECT_TRUE(a == b);
	EXPECT_FALSE(a != b);

}

TEST(DiffContainer, Boundary) {

	std::array<uint8_t, 7> const src = { 0, 1, 3, 6, 3, 1, 0 };
	auto const diff = 7;
	auto dc = math::MakeDiffContainer<int8_t>(std::begin(src), std::end(src),
			diff);

	auto const a = std::begin(dc), b = std::end(dc);

	EXPECT_EQ(0, std::distance(a, b));
	EXPECT_TRUE(a == b);
	EXPECT_FALSE(a != b);

}

TEST(DiffContainer, OutOfBoundary) {

	std::array<uint8_t, 7> const src = { 0, 1, 3, 6, 3, 1, 0 };
	auto const diff = 8;
	auto dc = math::MakeDiffContainer<int8_t>(std::begin(src), std::end(src),
			diff);

	auto const a = std::begin(dc), b = std::end(dc);

	EXPECT_EQ(0, std::distance(a, b));
	EXPECT_TRUE(a == b);
	EXPECT_FALSE(a != b);

}

TEST(DiffContainer, ZeroShift) {

	std::array<uint8_t, 7> const src = { 0, 1, 3, 6, 3, 1, 0 };
	auto const diff = 0;
	auto dc = math::MakeDiffContainer<int8_t>(std::begin(src), std::end(src),
			diff);

	auto const a = std::begin(dc), b = std::end(dc);

	EXPECT_EQ(7, std::distance(a, b));
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
	EXPECT_EQ(0, *a);

}

TEST(DiffContainer, IntOverflow) {

	std::array<uint8_t, 3> const src = { 255, 0, 255 };
	auto const diff = 1;
	auto dc = math::MakeDiffContainer<int16_t>(std::begin(src), std::end(src),
			diff);

	auto a = std::begin(dc);
	auto const b = std::end(dc);

	EXPECT_EQ(2, std::distance(a, b));

	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
	EXPECT_EQ(-255, *a);

	std::advance(a, 1);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
	EXPECT_EQ(255, *a);

	std::advance(a, 1);
	EXPECT_TRUE(a == b);
	EXPECT_FALSE(a != b);

}

}
