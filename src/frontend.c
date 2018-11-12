#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <CAENDigitizer.h>

#define ERREXIT() \
		if (handle) {	\
			CAEN_DGTZ_CloseDigitizer(handle);	\
		}	\
		exit(1)

#define CHECK(ret, msg)	\
	if (ret) {	\
		printf("Error %d %s\n", (int) ret, msg);	\
		ERREXIT();	\
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

	printf("***************************\n");
	printf("CAEN Digitizer Model: %s\n", boardInfo.ModelName);
	printf("Serial No: %d\n", boardInfo.SerialNumber);
	printf("Number of channels: %d\n", boardInfo.Channels);
	printf("Form-factor: %s\n", boardInfo.FormFactor == CAEN_DGTZ_VME64_FORM_FACTOR ? "VME64" : boardInfo.FormFactor == CAEN_DGTZ_VME64X_FORM_FACTOR ? "VME64X" : "Other");
	printf("Number of ADC bits: %d\n", boardInfo.ADC_NBits);
	printf("ROC FPGA Release: %s\n", boardInfo.ROC_FirmwareRel);
	printf("AMC FPGA Release: %s\n", boardInfo.AMC_FirmwareRel);
	printf("PCB Revision No: %d\n", boardInfo.PCB_Revision);
	printf("COMM Handle: %x\n", boardInfo.CommHandle);
	printf("VME Handle: %x\n", boardInfo.VMEHandle);
	printf("***************************\n");

	if (boardInfo.FamilyCode != CAEN_DGTZ_XX720_FAMILY_CODE) {
		printf("The device is not of CAEN x720 family\n");
		ERREXIT();
	}

	if (boardInfo.Model != CAEN_DGTZ_V1720) {
		printf("The device is not CAEN V1720\n");
		ERREXIT();
	}

	ret = CAEN_DGTZ_Reset(handle);
	CHECK(ret, "resetting digitizer");

	ret = CAEN_DGTZ_SetDPPAcquisitionMode(handle, CAEN_DGTZ_DPP_ACQ_MODE_Mixed,
			CAEN_DGTZ_DPP_SAVE_PARAM_None);
	CHECK(ret, "setting DPP acquisition mode");

	CAEN_DGTZ_CloseDigitizer(handle);
	return 0;
}
