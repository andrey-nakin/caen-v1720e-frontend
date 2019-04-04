#include <util/caen/V1724InfoRawData.hxx>
#include <gtest/gtest.h>

namespace util {

namespace caen {

TEST(V1724InfoRawData, TimeStamp) {

	{
		InfoBank bank;
		bank.timeStamp = 0x12345678;
		V1724InfoRawData const e(sizeof(bank), TID_DWORD, V1724InfoRawData::bankName(), &bank);
		EXPECT_EQ(0x12345678, e.timeStamp());
	}

	{
		InfoBank bank;
		bank.timeStamp = 0xffffffff;
		V1724InfoRawData const e(sizeof(bank), TID_DWORD, V1724InfoRawData::bankName(), &bank);
		EXPECT_EQ(0xffffffff, e.timeStamp());
	}

}

}

}
