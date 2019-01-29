#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
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

#ifndef min
#define min(a, b) (a) <= (b) ? (a) : (b)
#endif

#ifndef max
#define max(a, b) (a) >= (b) ? (a) : (b)
#endif

int const linkNum = 0;
int const conetNode = 0;
uint32_t const vmeBaseAddr = 0;
uint32_t const channelMask = 0xff;
uint32_t const triggerChannel = 0;
uint32_t const recordLength = 1024 * 1;	//	num of samples
uint32_t const postTrigger = 1024 * 1 - 64;	//	num of samples
uint8_t const IRQlevel = 1;
uint32_t const IRQstatusId = 1;
uint16_t const desiredNumOfEvents = 10;

uint32_t const REG_CHANNEL_CONFIG = 0x8000;
uint32_t const REG_POST_TRIGGER = 0x8114;

uint32_t const REG_BIT_TRIGGER_OVERLAP = 0x0001 << 1;

unsigned maxNumOfEvents = 0;

int main(int argc, char* argv[]) {
	CAEN_DGTZ_ErrorCode ret;
	CAEN_DGTZ_BoardInfo_t boardInfo;
	CAEN_DGTZ_EventInfo_t eventInfo;
	void *evt;
	int handle;
	uint32_t i, j, k;
	int majorNumber;
	char *buffer, *evtptr = NULL;
	uint32_t bufferSize, dataSize, numEvents, eventCounter;
	FILE *file;
	uint32_t maxEvent = 100;
	time_t now, last = 0;
	int verbose = 0;
	uint32_t regData;
	int waveformWritten = 0;

	if (argc > 1) {
		maxEvent = atoi(argv[1]);
	}

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

	sscanf(boardInfo.AMC_FirmwareRel, "%d", &majorNumber);
	if (majorNumber >= 128) {
		printf("This digitizer has a DPP firmware\n");
		ERREXIT();
	}

	ret = CAEN_DGTZ_Reset(handle);
	CHECK(ret, "resetting digitizer");

	ret = CAEN_DGTZ_SetIOLevel(handle, CAEN_DGTZ_IOLevel_NIM);
	CHECK(ret, "setting IO level");

	ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
	CHECK(ret, "setting external trigger input mode");

	ret = CAEN_DGTZ_SetChannelEnableMask(handle, channelMask);
	CHECK(ret, "setting channel enable mask");

	ret = CAEN_DGTZ_SetRunSynchronizationMode(handle,
			CAEN_DGTZ_RUN_SYNC_Disabled);
	CHECK(ret, "setting run sync mode");

	for (i = 0; i < boardInfo.Channels; i++) {
		if (channelMask & (1 << i)) {
			// Set the number of samples for each waveform (you can set different RL for different channels)
			ret = CAEN_DGTZ_SetRecordLength(handle, recordLength, i);
			CHECK(ret, "setting record length");

			// Set a DC offset to the input signal to adapt it to digitizer's dynamic range
			ret = CAEN_DGTZ_SetChannelDCOffset(handle, i, 0x8000);
			CHECK(ret, "setting channel DC offset");
		}
	}

	ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY,
			(1 << triggerChannel));
	CHECK(ret, "setting channel self trigger");

	ret = CAEN_DGTZ_SetChannelTriggerThreshold(handle, triggerChannel, 1000);
	CHECK(ret, "setting channel trigger threshold");

	ret = CAEN_DGTZ_SetTriggerPolarity(handle, triggerChannel,
			CAEN_DGTZ_TriggerOnFallingEdge);
	CHECK(ret, "setting trigger polarity");

	ret = CAEN_DGTZ_SetMaxNumEventsBLT(handle, 10);
	CHECK(ret, "setting max num events");

	ret = CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
	CHECK(ret, "setting acquisition mode");

	ret = CAEN_DGTZ_ReadRegister(handle, REG_CHANNEL_CONFIG, &regData);
	CHECK(ret, "reading channel config register");
	if (regData & REG_BIT_TRIGGER_OVERLAP) {
		// disable trigger overlap
		ret = CAEN_DGTZ_WriteRegister(handle, REG_CHANNEL_CONFIG,
				regData & ~REG_BIT_TRIGGER_OVERLAP);
		CHECK(ret, "writing channel config register");
	}

	ret = CAEN_DGTZ_WriteRegister(handle, REG_POST_TRIGGER, postTrigger / 4);
	CHECK(ret, "writing post trigger register");
	ret = CAEN_DGTZ_ReadRegister(handle, REG_POST_TRIGGER, &regData);
	CHECK(ret, "reading post trigger register");
	printf("Value of post trigger register: %" PRIu32 "\n", regData);

	ret = CAEN_DGTZ_ReadRegister(handle, 0x800C, &regData);
	CHECK(ret, "reading 0x800C register");
	printf("Value of 0x800C register: %" PRIu32 "\n", regData);

	ret = CAEN_DGTZ_ReadRegister(handle, 0x8020, &regData);
	CHECK(ret, "reading 0x8020 register");
	printf("Value of 0x8020 register: %" PRIu32 "\n", regData);

	ret = CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &bufferSize);
	CHECK(ret, "allocating readout buffer");
	printf("Readout buffer size: %" PRIu32 "\n", bufferSize);

	ret = CAEN_DGTZ_AllocateEvent(handle, &evt);
	CHECK(ret, "allocating event");

	file = fopen("data", "w+");
	if (!file) {
		printf("Cannot open file for write\n");
		ERREXIT();
	}

	ret = CAEN_DGTZ_SWStartAcquisition(handle);
	CHECK(ret, "starting acquisition");

	for (eventCounter = 0; eventCounter < maxEvent;) {
		ret = CAEN_DGTZ_ReadData(handle,
				CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &dataSize);
		CHECK(ret, "reading data");

		ret = CAEN_DGTZ_GetNumEvents(handle, buffer, dataSize, &numEvents);
		CHECK(ret, "getting num events");
		maxNumOfEvents = max(maxNumOfEvents, numEvents);

		if (numEvents > 0) {
			now = time(NULL);
			if (now > last) {
				verbose = 1;
				last = now;
			} else {
				verbose = 0;
			}

			for (i = 0; i < numEvents; i++, eventCounter++) {
				ret = CAEN_DGTZ_GetEventInfo(handle, buffer, dataSize, i,
						&eventInfo, &evtptr);
				CHECK(ret, "getting event info");

				if (verbose) {
					printf(
							"Event #=%" PRIu32 ", size=%" PRIu32 ", channels=%" PRIx32
							", time=%" PRIu32 ", max # of events=%u\n",
							eventInfo.EventCounter, eventInfo.EventSize,
							eventInfo.ChannelMask,
							eventInfo.TriggerTimeTag & ~0x80000000,
							maxNumOfEvents);
				}

				ret = CAEN_DGTZ_DecodeEvent(handle, evtptr, &evt);
				CHECK(ret, "decoding event");

				//*************************************
				// Event Elaboration
				//*************************************

				for (j = 0; j < boardInfo.Channels; j++) {
					if ((1 << j) & eventInfo.ChannelMask) {
						uint32_t const numOfSamples =
								((CAEN_DGTZ_UINT16_EVENT_t*) evt)->ChSize[j];
						uint16_t const *samples =
								((CAEN_DGTZ_UINT16_EVENT_t*) evt)->DataChannel[j];
						uint16_t minSample = 0, maxSample = 0;
						uint32_t minPos, maxPos = 0;

						if (numOfSamples > 0) {
							minSample = maxSample = samples[0];

							for (k = numOfSamples - 1; k > 0; k--) {
								uint16_t const s = samples[k];
								if (minSample > s) {
									minSample = s;
									minPos = k;
								}
								if (maxSample < s) {
									maxSample = s;
									maxPos = k;
								}
							}

							if (j == 0 && !waveformWritten) {
								// write waveform on channel #0 to a text file

								for (k = 0; k < numOfSamples; k++) {
									fprintf(file, "%" PRIu16 "\n", samples[k]);
								}

								waveformWritten = 1;
							}

						}

						if (verbose) {
							printf("Channel %" PRIu32 ", # samples=%" PRIu16
							", min = %" PRIu16 " at %" PRIu32
							", max = %" PRIu16 " at %" PRIu32 "\n", j,
									numOfSamples, minSample, minPos, maxSample,
									maxPos);
						}
					}
				}
			}
		}
	}

	fclose(file);

	ret = CAEN_DGTZ_FreeEvent(handle, &evt);
	CHECK(ret, "freeing event");

	ret = CAEN_DGTZ_FreeReadoutBuffer(&buffer);
	CHECK(ret, "freeing readout buffer");

	ret = CAEN_DGTZ_Reset(handle);
	CHECK(ret, "resetting digitizer");

	CAEN_DGTZ_CloseDigitizer(handle);
	return 0;
}
