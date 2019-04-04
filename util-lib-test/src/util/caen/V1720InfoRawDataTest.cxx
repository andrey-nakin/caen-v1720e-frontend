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

TEST(V1720InfoRawData, TimeStampDifferenceInNs) {

	{
		InfoBank bank;
		bank.timeStamp = 123;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(3 * 2 * 4, e.timeStampDifferenceInNs(120));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0x00000000ul;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(3 * 2 * 4, e.timeStampDifferenceInNs(0x7ffffffdul));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0x00000002ul;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(3 * 2 * 4, e.timeStampDifferenceInNs(0x7ffffffful));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0x80000000ul;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(3 * 2 * 4, e.timeStampDifferenceInNs(0xfffffffdul));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0x80000002ul;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(3 * 2 * 4, e.timeStampDifferenceInNs(0xfffffffful));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0x7ffffffful;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(0x7fffffffull * 2 * 4, e.timeStampDifferenceInNs(0x00000000ul));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0xfffffffful;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(0x7fffffffull * 2 * 4, e.timeStampDifferenceInNs(0x00000000ul));
	}
}

TEST(V1720InfoRawData, TimeStampDifferenceInSamples) {

	{
		InfoBank bank;
		bank.timeStamp = 123;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(3 * 2, e.timeStampDifferenceInSamples(120));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0x00000000ul;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(3 * 2, e.timeStampDifferenceInSamples(0x7ffffffdul));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0x00000002ul;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(3 * 2, e.timeStampDifferenceInSamples(0x7ffffffful));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0x80000000ul;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(3 * 2, e.timeStampDifferenceInSamples(0xfffffffdul));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0x80000002ul;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(3 * 2, e.timeStampDifferenceInSamples(0xfffffffful));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0x7ffffffful;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(0x7fffffffull * 2 , e.timeStampDifferenceInSamples(0x00000000ul));
	}

	{
		InfoBank bank;
		bank.timeStamp = 0xfffffffful;
		V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
		EXPECT_EQ(0x7fffffffull * 2, e.timeStampDifferenceInSamples(0x00000000ul));
	}
}

TEST(V1720InfoRawData, SampleWidthInBits) {

	InfoBank bank;
	V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
	EXPECT_EQ(12, e.sampleWidthInBits());

}

}

}
