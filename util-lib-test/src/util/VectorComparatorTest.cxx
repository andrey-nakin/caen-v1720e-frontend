#include <gtest/gtest.h>
#include <util/VectorComparator.hxx>

namespace util {

TEST(VectorComparator, Generic) {

	std::vector<int> const a = {1, 2, 3, 4, 5};
	std::vector<int> const b = {1, 2, 3, 4, 5};
	std::vector<int> const c = {1, 2, 3, 4, 6};

	EXPECT_TRUE(VectorComparator::equal(a, b));
	EXPECT_TRUE(VectorComparator::equal(b, a));
	EXPECT_FALSE(VectorComparator::equal(a, c));
	EXPECT_FALSE(VectorComparator::equal(c, a));
	EXPECT_FALSE(VectorComparator::equal(b, c));
	EXPECT_FALSE(VectorComparator::equal(c, b));

}

TEST(VectorComparator, Empty) {

	std::vector<int> const a;
	std::vector<int> const b;

	EXPECT_TRUE(VectorComparator::equal(a, b));
	EXPECT_TRUE(VectorComparator::equal(b, a));

}

TEST(VectorComparator, DifferentSize) {

	std::vector<int> const a = {1, 2, 3, 4, 5};
	std::vector<int> const b = {1, 2, 3, 4, 5, 6};

	EXPECT_FALSE(VectorComparator::equal(a, b));
	EXPECT_FALSE(VectorComparator::equal(b, a));

}

TEST(VectorComparator, GenericBool) {

	std::vector<bool> const a = {true, false, true, false, true};
	std::vector<bool> const b = {true, false, true, false, true};
	std::vector<bool> const c = {true, false, true, false, false};

	EXPECT_TRUE(VectorComparator::equal(a, b));
	EXPECT_TRUE(VectorComparator::equal(b, a));
	EXPECT_FALSE(VectorComparator::equal(a, c));
	EXPECT_FALSE(VectorComparator::equal(c, a));
	EXPECT_FALSE(VectorComparator::equal(b, c));
	EXPECT_FALSE(VectorComparator::equal(c, b));

}

TEST(VectorComparator, EmptyBool) {

	std::vector<bool> const a;
	std::vector<bool> const b;

	EXPECT_TRUE(VectorComparator::equal(a, b));
	EXPECT_TRUE(VectorComparator::equal(b, a));

}

TEST(VectorComparator, DifferentSizeBool) {

	std::vector<bool> const a = {true, false, true, false, true};
	std::vector<bool> const b = {true, false, true, false, true, false};

	EXPECT_FALSE(VectorComparator::equal(a, b));
	EXPECT_FALSE(VectorComparator::equal(b, a));

}

}
