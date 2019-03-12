#include <array>
#include <iterator>
#include <gtest/gtest.h>
#include <math/EdgeFinder.hxx>

namespace math {

TEST(EdgeFinder, RisingEqual) {

	std::array<int, 5> const src = { 1, 2, 3, 4, 5 };
	auto i = FindEdge(true, 3, std::begin(src), std::end(src));

	EXPECT_EQ(4, *i);
	EXPECT_EQ(3, std::distance(std::begin(src), i));

}

TEST(EdgeFinder, RisingNotEqual) {

	std::array<int, 5> const src = { 2, 4, 6, 8, 10 };
	auto i = FindEdge(true, 5, std::begin(src), std::end(src));

	EXPECT_EQ(6, *i);
	EXPECT_EQ(2, std::distance(std::begin(src), i));

}

TEST(EdgeFinder, RisingNotExceeded) {

	std::array<int, 5> const src = { 1, 2, 3, 4, 5 };
	auto i = FindEdge(true, 5, std::begin(src), std::end(src));

	EXPECT_EQ(std::end(src), i);

}

TEST(EdgeFinder, RisingOverEqual) {

	std::array<int, 10> const src = { 8, 6, 4, 2, 0, 2, 4, 6, 8, 10 };
	auto i = FindEdge(true, 4, std::begin(src), std::end(src));

	EXPECT_EQ(6, *i);
	EXPECT_EQ(7, std::distance(std::begin(src), i));

}

TEST(EdgeFinder, RisingOverNotEqual) {

	std::array<int, 10> const src = { 8, 6, 4, 2, 0, 2, 4, 6, 8, 10 };
	auto i = FindEdge(true, 5, std::begin(src), std::end(src));

	EXPECT_EQ(6, *i);
	EXPECT_EQ(7, std::distance(std::begin(src), i));

}

TEST(EdgeFinder, RisingSingle) {

	std::array<int, 1> const src = { 5 };
	auto i = FindEdge(true, 5, std::begin(src), std::end(src));

	EXPECT_EQ(std::end(src), i);

}

TEST(EdgeFinder, RisingEmpty) {

	std::array<int, 0> const src = { };
	auto i = FindEdge(true, 5, std::begin(src), std::end(src));

	EXPECT_EQ(std::end(src), i);

}

TEST(EdgeFinder, FallingEqual) {

	std::array<int, 5> const src = { 5, 4, 3, 2, 1 };
	auto i = FindEdge(false, 3, std::begin(src), std::end(src));

	EXPECT_EQ(2, *i);
	EXPECT_EQ(3, std::distance(std::begin(src), i));

}

TEST(EdgeFinder, FallingNotEqual) {

	std::array<int, 5> const src = { 10, 8, 6, 4, 2 };
	auto i = FindEdge(false, 7, std::begin(src), std::end(src));

	EXPECT_EQ(6, *i);
	EXPECT_EQ(2, std::distance(std::begin(src), i));

}

TEST(EdgeFinder, FallingNotExceeded) {

	std::array<int, 4> const src = { 5, 4, 3, 2 };
	auto i = FindEdge(false, 2, std::begin(src), std::end(src));

	EXPECT_EQ(std::end(src), i);

}

TEST(EdgeFinder, FallingOverEqual) {

	std::array<int, 10> const src = { 2, 4, 6, 8, 10, 8, 6, 4, 2 };
	auto i = FindEdge(false, 8, std::begin(src), std::end(src));

	EXPECT_EQ(6, *i);
	EXPECT_EQ(6, std::distance(std::begin(src), i));

}

TEST(EdgeFinder, FallingOverNotEqual) {

	std::array<int, 10> const src = { 2, 4, 6, 8, 10, 8, 6, 4, 2 };
	auto i = FindEdge(false, 7, std::begin(src), std::end(src));

	EXPECT_EQ(6, *i);
	EXPECT_EQ(6, std::distance(std::begin(src), i));

}

TEST(EdgeFinder, FallingSingle) {

	std::array<int, 1> const src = { 5 };
	auto i = FindEdge(false, 5, std::begin(src), std::end(src));

	EXPECT_EQ(std::end(src), i);

}

TEST(EdgeFinder, FallingEmpty) {

	std::array<int, 0> const src = { };
	auto i = FindEdge(false, 5, std::begin(src), std::end(src));

	EXPECT_EQ(std::end(src), i);

}

}
