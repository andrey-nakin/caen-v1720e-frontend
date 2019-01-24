#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <math/PeakFinder.hxx>

namespace math {

TEST(PeakFinder, Generic) {

	std::array<uint16_t, 50> const src = {
		2147, 2148, 2151, 2149, 2145, 2150, 2149, 2152, 2151, 2149,
		2149, 2146, 2148, 2151, 2150, 2149, 2149, 2147, 2147, 2144,
		2135, 2117, 2113, 2124, 2133, 2143, 2143, 2144, 2149, 2153,
		2148, 2149, 2150, 2150, 2150, 2150, 2149, 2149, 2150, 2149,
		2147, 2146, 2148, 2146, 2151, 2148, 2148, 2149, 2149, 2148
	};
	auto pf = MakePeakFinder(false, std::begin(src), std::end(src), 3, 15, 10);

	EXPECT_TRUE(pf.HasNext());
	auto i = pf.GetNext();
	EXPECT_EQ(22, std::distance(std::begin(src), i));
	EXPECT_EQ(2113, *i);

	EXPECT_FALSE(pf.HasNext());

}

}
