#include <cstring>
#include <util/types.hxx>
#include <gtest/gtest.h>

namespace util {

TEST(FillTriggerInfo, Generic) {

	constexpr int FILLER = -1;

	union {
		TriggerBank t;
		struct {
			uint16_t w0, w1, w2, w3;
		}words;
	}bank;

	std::memset(&bank, FILLER, sizeof(bank));
	fillTriggerInfo(bank.t, 0x0f, 0x12345678, false, false);
	EXPECT_EQ(0x000f, bank.words.w0);
	EXPECT_EQ(0x5678, bank.words.w1);
	EXPECT_EQ(0x0000, bank.words.w2);
	EXPECT_EQ(0x1234, bank.words.w3);

	std::memset(&bank, FILLER, sizeof(bank));
	fillTriggerInfo(bank.t, 0x0f, 0x12345678, true, false);
	EXPECT_EQ(0x000f, bank.words.w0);
	EXPECT_EQ(0x5678, bank.words.w1);
	EXPECT_EQ(0x0001, bank.words.w2);
	EXPECT_EQ(0x1234, bank.words.w3);

	std::memset(&bank, FILLER, sizeof(bank));
	fillTriggerInfo(bank.t, 0x0f, 0x12345678, false, true);
	EXPECT_EQ(0x000f, bank.words.w0);
	EXPECT_EQ(0x5678, bank.words.w1);
	EXPECT_EQ(0x0002, bank.words.w2);
	EXPECT_EQ(0x1234, bank.words.w3);

	std::memset(&bank, FILLER, sizeof(bank));
	fillTriggerInfo(bank.t, 0x0f, 0x12345678, true, true);
	EXPECT_EQ(0x000f, bank.words.w0);
	EXPECT_EQ(0x5678, bank.words.w1);
	EXPECT_EQ(0x0003, bank.words.w2);
	EXPECT_EQ(0x1234, bank.words.w3);

}

}
