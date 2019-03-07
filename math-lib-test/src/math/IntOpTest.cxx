#include <gtest/gtest.h>
#include <math/IntOp.hxx>

namespace math {

TEST(IntOp, Module) {

	typedef IntOp<uint32_t, 31> Engine;
	EXPECT_EQ(0x80000000, Engine::module());

}

TEST(IntOp, Mask) {

	typedef IntOp<uint32_t, 31> Engine;
	EXPECT_EQ(0x7fffffff, Engine::mask());

}

TEST(IntOp, Value) {

	typedef IntOp<uint32_t, 31> Engine;
	EXPECT_EQ(0x7fffffff, Engine::value(0xffffffff));
	EXPECT_EQ(0x7fffffff, Engine::value(0x7fffffff));
	EXPECT_EQ(0x0fffffff, Engine::value(0x0fffffff));

}

TEST(IntOp, Sub) {

	typedef IntOp<uint32_t, 31> Engine;
	EXPECT_EQ(0x00000001, Engine::sub(0x12345678, 0x12345677));
	EXPECT_EQ(0x7fffffff, Engine::sub(0x12345677, 0x12345678));
	EXPECT_EQ(
			0x12345,
			Engine::sub(
					static_cast<uint32_t>(0x7fff0000) + static_cast<uint32_t>(0x12345),
					0x7fff0000
			)
	);

}

}
