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

TEST(V1720InfoRawData, DcMultiplier) {

	InfoBank bank;
	V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);
	EXPECT_NEAR(4.884005E-004, e.dcMultiplier(), 0.5e-10);

}

TEST(V1720InfoRawData, DcBaseline) {

	InfoBank bank;
	V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);

	EXPECT_NEAR(-2.00, e.dcBaseline(0x0000u), 0.5e-10);
	EXPECT_NEAR(-1.00, e.dcBaseline(0x7fffu), 0.5e-4);
	EXPECT_NEAR(-1.00, e.dcBaseline(0x8000u), 0.5e-4);
	EXPECT_NEAR(0.00, e.dcBaseline(0xffffu), 0.5e-10);
}

TEST(V1720InfoRawData, DcValue) {

	InfoBank bank;
	V1720InfoRawData const e(sizeof(bank), TID_DWORD, V1720InfoRawData::bankName(), &bank);

	EXPECT_NEAR(-2.00, e.dcValue(0u, 0x0000u), 0.5e-10);
	EXPECT_NEAR(0.00, e.dcValue(0u, 0xffffu), 0.5e-10);

	EXPECT_NEAR(0.00, e.dcValue(4095u, 0x0000u), 0.5e-10);
	EXPECT_NEAR(2.00, e.dcValue(4095u, 0xffffu), 0.5e-10);
}

}

}
