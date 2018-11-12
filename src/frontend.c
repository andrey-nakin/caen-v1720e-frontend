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
uint32_t const channelMask = 0xff;
uint32_t const triggerChannel = 0;

int main(int argc, char* argv[]) {
	CAEN_DGTZ_ErrorCode ret;
	CAEN_DGTZ_BoardInfo_t boardInfo;
	int handle;
	int i;

	ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, linkNum, conetNode,
			vmeBaseAddr, &handle);
	CHECK(ret, "opening digitizer");

	ret = CAEN_DGTZ_GetInfo(handle, &boardInfo);
	CHECK(ret, "getting digitizer info");

	printf("***************************\n");
	printf("CAEN Digitizer Model: %s\n", boardInfo.ModelName);
	printf("Serial No: %d\n", boardInfo.SerialNumber);
	printf("Number of channels: %d\n", boardInfo.Channels);
	printf("Form-factor: %s\n",
			boardInfo.FormFactor == CAEN_DGTZ_VME64_FORM_FACTOR ? "VME64" :
			boardInfo.FormFactor == CAEN_DGTZ_VME64X_FORM_FACTOR ?
					"VME64X" : "Other");
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

//	ret = CAEN_DGTZ_SetDPPAcquisitionMode(handle, CAEN_DGTZ_DPP_ACQ_MODE_Mixed,
//			CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
//	CHECK(ret, "setting DPP acquisition mode");

	ret = CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
	CHECK(ret, "setting acquisition mode");

	ret = CAEN_DGTZ_SetIOLevel(handle, CAEN_DGTZ_IOLevel_NIM);
	CHECK(ret, "setting IO level");

	ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
	CHECK(ret, "setting external trigger input mode");

	ret = CAEN_DGTZ_SetChannelEnableMask(handle, channelMask);
	CHECK(ret, "setting channel enable mask");

//    ret = CAEN_DGTZ_SetDPPEventAggregation(handle, 0, 0);
//	CHECK(ret, "setting DPP event aggregation");

	ret = CAEN_DGTZ_SetRunSynchronizationMode(handle,
			CAEN_DGTZ_RUN_SYNC_Disabled);
	CHECK(ret, "setting run sync mode");

	for (i = 0; i < boardInfo.Channels; i++) {
		if (channelMask & (1 << i)) {
			// Set the number of samples for each waveform (you can set different RL for different channels)
			ret = CAEN_DGTZ_SetRecordLength(handle, 1000, i);
			CHECK(ret, "setting recond length");

			// Set a DC offset to the input signal to adapt it to digitizer's dynamic range
			ret = CAEN_DGTZ_SetChannelDCOffset(handle, i, 0x8000);
			CHECK(ret, "setting channel DC offset");

			// Set the Pre-Trigger size (in samples of 4 ns each)
//			ret = CAEN_DGTZ_SetDPPPreTriggerSize(handle, i, 80);
//			CHECK(ret, "setting pre-trigger size");

			// Set the polarity for the given channel
			ret = CAEN_DGTZ_SetChannelPulsePolarity(handle, i,
					CAEN_DGTZ_PulsePolarityNegative);
			CHECK(ret, "setting pulse polarity");
		}
	}

	CAEN_DGTZ_CloseDigitizer(handle);
	return 0;
}
