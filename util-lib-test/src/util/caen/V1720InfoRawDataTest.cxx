#include <util/caen/V1720InfoRawData.hxx>
#include <gtest/gtest.h>

namespace util {

namespace caen {

TEST(V1720InfoRawData, TimeStamp) {

	{
		InfoBank bank;
		bank.timeStamp = 0x12345678;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(0x12345678, e.timeStamp());
	}

	{
		InfoBank bank;
		bank.timeStamp = 0xffffffff;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(0xffffffff, e.timeStamp());
	}

}

}

}
