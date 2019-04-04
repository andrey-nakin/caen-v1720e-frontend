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

TEST(V1724InfoRawData, TimeStampDifferenceInNs) {

	{
		InfoBank bank;
		bank.timeStamp = 123;
		V1724InfoRawData const e(sizeof(bank), TID_DWORD, V1724InfoRawData::bankName(), &bank);
		EXPECT_EQ(3 * 10, e.timeStampDifferenceInNs(120));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0x00000000ul;
		V1724InfoRawData const e(sizeof(bank), TID_DWORD, V1724InfoRawData::bankName(), &bank);
		EXPECT_EQ(3 * 10, e.timeStampDifferenceInNs(0x7ffffffdul));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0x00000002ul;
		V1724InfoRawData const e(sizeof(bank), TID_DWORD, V1724InfoRawData::bankName(), &bank);
		EXPECT_EQ(3 * 10, e.timeStampDifferenceInNs(0x7ffffffful));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0x80000000ul;
		V1724InfoRawData const e(sizeof(bank), TID_DWORD, V1724InfoRawData::bankName(), &bank);
		EXPECT_EQ(3 * 10, e.timeStampDifferenceInNs(0xfffffffdul));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0x80000002ul;
		V1724InfoRawData const e(sizeof(bank), TID_DWORD, V1724InfoRawData::bankName(), &bank);
		EXPECT_EQ(3 * 10, e.timeStampDifferenceInNs(0xfffffffful));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0x7ffffffful;
		V1724InfoRawData const e(sizeof(bank), TID_DWORD, V1724InfoRawData::bankName(), &bank);
		EXPECT_EQ(0x7fffffffull * 10, e.timeStampDifferenceInNs(0x00000000ul));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0xfffffffful;
		V1724InfoRawData const e(sizeof(bank), TID_DWORD, V1724InfoRawData::bankName(), &bank);
		EXPECT_EQ(0x7fffffffull * 10, e.timeStampDifferenceInNs(0x00000000ul));
	}
}

}

}
