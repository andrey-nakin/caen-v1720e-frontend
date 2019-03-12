#include <array>
#include <iterator>
#include <gtest/gtest.h>
#include <math/EdgeFinder.hxx>

namespace math {

TEST(FindEdgeDistance, RisingEqual) {

	std::array<int, 5> const src = { 1, 2, 3, 4, 5 };
	auto i = FindEdgeDistance(true, 3, std::begin(src), std::end(src));

	EXPECT_EQ(3, i);

}

TEST(FindEdgeDistance, RisingNotEqual) {

	std::array<int, 5> const src = { 2, 4, 6, 8, 10 };
	auto i = FindEdgeDistance(true, 5, std::begin(src), std::end(src));

	EXPECT_EQ(2, i);

}

TEST(FindEdgeDistance, RisingNotExceeded) {

	std::array<int, 5> const src = { 1, 2, 3, 4, 5 };
	auto i = FindEdgeDistance(true, 5, std::begin(src), std::end(src));

	EXPECT_EQ(0, i);

}

TEST(FindEdgeDistance, RisingOverEqual) {

	std::array<int, 10> const src = { 8, 6, 4, 2, 0, 2, 4, 6, 8, 10 };
	auto i = FindEdgeDistance(true, 4, std::begin(src), std::end(src));

	EXPECT_EQ(7, i);

}

TEST(FindEdgeDistance, RisingOverNotEqual) {

	std::array<int, 10> const src = { 8, 6, 4, 2, 0, 2, 4, 6, 8, 10 };
	auto i = FindEdgeDistance(true, 5, std::begin(src), std::end(src));

	EXPECT_EQ(7, i);

}

TEST(FindEdgeDistance, RisingSingle) {

	std::array<int, 1> const src = { 5 };
	auto i = FindEdgeDistance(true, 5, std::begin(src), std::end(src));

	EXPECT_EQ(0, i);

}

TEST(FindEdgeDistance, RisingEmpty) {

	std::array<int, 0> const src = { };
	auto i = FindEdgeDistance(true, 5, std::begin(src), std::end(src));

	EXPECT_EQ(0, i);

}

TEST(FindEdgeDistance, FallingEqual) {

	std::array<int, 5> const src = { 5, 4, 3, 2, 1 };
	auto i = FindEdgeDistance(false, 3, std::begin(src), std::end(src));

	EXPECT_EQ(3, i);

}

TEST(FindEdgeDistance, FallingNotEqual) {

	std::array<int, 5> const src = { 10, 8, 6, 4, 2 };
	auto i = FindEdgeDistance(false, 7, std::begin(src), std::end(src));

	EXPECT_EQ(2, i);

}

TEST(FindEdgeDistance, FallingNotExceeded) {

	std::array<int, 4> const src = { 5, 4, 3, 2 };
	auto i = FindEdgeDistance(false, 2, std::begin(src), std::end(src));

	EXPECT_EQ(0, i);

}

TEST(FindEdgeDistance, FallingOverEqual) {

	std::array<int, 10> const src = { 2, 4, 6, 8, 10, 8, 6, 4, 2 };
	auto i = FindEdgeDistance(false, 8, std::begin(src), std::end(src));

	EXPECT_EQ(6, i);

}

TEST(FindEdgeDistance, FallingOverNotEqual) {

	std::array<int, 10> const src = { 2, 4, 6, 8, 10, 8, 6, 4, 2 };
	auto i = FindEdgeDistance(false, 7, std::begin(src), std::end(src));

	EXPECT_EQ(6, i);

}

TEST(FindEdgeDistance, FallingSingle) {

	std::array<int, 1> const src = { 5 };
	auto i = FindEdgeDistance(false, 5, std::begin(src), std::end(src));

	EXPECT_EQ(0, i);

}

TEST(FindEdgeDistance, FallingEmpty) {

	std::array<int, 0> const src = { };
	auto i = FindEdgeDistance(false, 5, std::begin(src), std::end(src));

	EXPECT_EQ(0, i);

}

}
