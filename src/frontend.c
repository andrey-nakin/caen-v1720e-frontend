#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <CAENDigitizer.h>

#define CHECK(ret, msg)	\
	if (ret) {	\
		printf("Error %d %s\n", (int) ret, msg);	\
		if (handle) {	\
			CAEN_DGTZ_CloseDigitizer(handle);	\
		}	\
		exit(1);	\
	}

int const linkNum = 0;
int const conetNode = 0;
uint32_t const vmeBaseAddr = 0;

int main(int argc, char* argv[]) {
	CAEN_DGTZ_ErrorCode ret;
	CAEN_DGTZ_BoardInfo_t boardInfo;
	int handle;

	ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, linkNum, conetNode,
			vmeBaseAddr, &handle);
	CHECK(ret, "opening digitizer");

	ret = CAEN_DGTZ_GetInfo(handle, &boardInfo);
	CHECK(ret, "getting digitizer info");

	printf("\nConnected to CAEN Digitizer Model %s\n", boardInfo.ModelName);
	printf("ROC FPGA Release is %s\n", boardInfo.ROC_FirmwareRel);
	printf("AMC FPGA Release is %s\n", boardInfo.AMC_FirmwareRel);

	CAEN_DGTZ_CloseDigitizer(handle);
	return 0;
}
