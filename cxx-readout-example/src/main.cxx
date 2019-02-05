#include <time.h>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <thread>
#include <CAENDigitizer.h>
#include <caen/device.hxx>

#define ERREXIT() \
		exit(1)

#define CHECK(ret, msg)	\
	if (ret) {	\
		std::cerr << "Error " << ret << " " << msg << std::endl; \
		ERREXIT();	\
	}

int const linkNum = 0;
int const conetNode = 0;
uint32_t const vmeBaseAddr = 0;
uint32_t const channelMask = 0xff;
uint32_t const triggerChannel = 0;
uint16_t const triggerThreshold = 1000; //2100;
uint32_t const recordLength = 1024 * 1;	//	num of samples
uint32_t const postTrigger = 1024 * 1 - 64;	//	num of samples
uint8_t const IRQlevel = 1;
uint32_t const IRQstatusId = 1;
uint16_t const desiredNumOfEvents = 10;

uint32_t const REG_CHANNEL_CONFIG = 0x8000;
uint32_t const REG_POST_TRIGGER = 0x8114;

uint32_t const REG_BIT_TRIGGER_OVERLAP = 0x0001 << 1;

int main(int argc, char* argv[]) {
	CAEN_DGTZ_ErrorCode ret;
	CAEN_DGTZ_BoardInfo_t boardInfo;
	uint32_t i, j;
	int majorNumber;
	uint32_t eventCounter;
	uint32_t maxEvent = 100;
	time_t now, last = 0;
	int verbose = 0;
	uint32_t regData;
	bool waveformWritten = false;

	if (argc > 1) {
		maxEvent = atoi(argv[1]);
	}

	try {
		caen::Device device(caen::Handle(linkNum, conetNode, vmeBaseAddr));

		ret = CAEN_DGTZ_GetInfo(device.getHandle(), &boardInfo);
		CHECK(ret, "getting digitizer info");

		std::cout << "***************************\n" << "CAEN Digitizer Model: "
				<< boardInfo.ModelName << "\n" << "Serial No: "
				<< boardInfo.SerialNumber << "\n" << "Number of channels: "
				<< boardInfo.Channels << "\n" << "Form-factor: "
				<< (boardInfo.FormFactor == CAEN_DGTZ_VME64_FORM_FACTOR ?
						"VME64" :
					boardInfo.FormFactor == CAEN_DGTZ_VME64X_FORM_FACTOR ?
							"VME64X" : "Other") << "\n"
				<< "Number of ADC bits: " << boardInfo.ADC_NBits << "\n"
				<< "ROC FPGA Release: " << boardInfo.ROC_FirmwareRel << "\n"
				<< "AMC FPGA Release: " << boardInfo.AMC_FirmwareRel << "\n"
				<< "PCB Revision No: " << boardInfo.PCB_Revision << "\n"
				<< "COMM device.getHandle(): " << boardInfo.CommHandle << "\n"
				<< "VME device.getHandle(): " << boardInfo.VMEHandle << "\n"
				<< "***************************" << std::endl;

		if (boardInfo.FamilyCode != CAEN_DGTZ_XX720_FAMILY_CODE) {
			std::cerr << "The device is not of CAEN x720 family\n";
			ERREXIT();
		}

		if (boardInfo.Model != CAEN_DGTZ_V1720) {
			std::cerr << "The device is not CAEN V1720\n";
			ERREXIT();
		}

		sscanf(boardInfo.AMC_FirmwareRel, "%d", &majorNumber);
		if (majorNumber >= 128) {
			std::cerr << "This digitizer has a DPP firmware\n";
			ERREXIT();
		}

		ret = CAEN_DGTZ_Reset(device.getHandle());
		CHECK(ret, "resetting digitizer");

		ret = CAEN_DGTZ_SetIOLevel(device.getHandle(), CAEN_DGTZ_IOLevel_NIM);
		CHECK(ret, "setting IO level");

		ret = CAEN_DGTZ_SetExtTriggerInputMode(device.getHandle(),
				CAEN_DGTZ_TRGMODE_ACQ_ONLY);
		CHECK(ret, "setting external trigger input mode");

		ret = CAEN_DGTZ_SetChannelEnableMask(device.getHandle(), channelMask);
		CHECK(ret, "setting channel enable mask");

		ret = CAEN_DGTZ_SetRunSynchronizationMode(device.getHandle(),
				CAEN_DGTZ_RUN_SYNC_Disabled);
		CHECK(ret, "setting run sync mode");

		for (i = 0; i < boardInfo.Channels; i++) {
			if (channelMask & (1 << i)) {
				// Set the number of samples for each waveform (you can set different RL for different channels)
				ret = CAEN_DGTZ_SetRecordLength(device.getHandle(),
						recordLength, i);
				CHECK(ret, "setting record length");

				// Set a DC offset to the input signal to adapt it to digitizer's dynamic range
				ret = CAEN_DGTZ_SetChannelDCOffset(device.getHandle(), i,
						0x8000);
				CHECK(ret, "setting channel DC offset");
			}
		}

		ret = CAEN_DGTZ_SetChannelSelfTrigger(device.getHandle(),
				CAEN_DGTZ_TRGMODE_ACQ_ONLY, (1 << triggerChannel));
		CHECK(ret, "setting channel self trigger");

		ret = CAEN_DGTZ_SetChannelTriggerThreshold(device.getHandle(),
				triggerChannel, triggerThreshold);
		CHECK(ret, "setting channel trigger threshold");

		ret = CAEN_DGTZ_SetTriggerPolarity(device.getHandle(), triggerChannel,
				CAEN_DGTZ_TriggerOnFallingEdge);
		CHECK(ret, "setting trigger polarity");

		ret = CAEN_DGTZ_SetMaxNumEventsBLT(device.getHandle(), 10);
		CHECK(ret, "setting max num events");

		ret = CAEN_DGTZ_SetAcquisitionMode(device.getHandle(),
				CAEN_DGTZ_SW_CONTROLLED);
		CHECK(ret, "setting acquisition mode");

		ret = CAEN_DGTZ_ReadRegister(device.getHandle(), REG_CHANNEL_CONFIG,
				&regData);
		CHECK(ret, "reading channel config register");
		if (regData & REG_BIT_TRIGGER_OVERLAP) {
			// disable trigger overlap
			ret = CAEN_DGTZ_WriteRegister(device.getHandle(),
					REG_CHANNEL_CONFIG, regData & ~REG_BIT_TRIGGER_OVERLAP);
			CHECK(ret, "writing channel config register");
		}

		ret = CAEN_DGTZ_WriteRegister(device.getHandle(), REG_POST_TRIGGER,
				postTrigger / 4);
		CHECK(ret, "writing post trigger register");
		ret = CAEN_DGTZ_ReadRegister(device.getHandle(), REG_POST_TRIGGER,
				&regData);
		CHECK(ret, "reading post trigger register");
		std::cout << "Value of post trigger register: " << regData << std::endl;

		ret = CAEN_DGTZ_ReadRegister(device.getHandle(), 0x800C, &regData);
		CHECK(ret, "reading 0x800C register");
		std::cout << "Value of 0x800C register: " << regData << std::endl;

		ret = CAEN_DGTZ_ReadRegister(device.getHandle(), 0x8020, &regData);
		CHECK(ret, "reading 0x8020 register");
		std::cout << "Value of 0x8020 register: " << regData << std::endl;

		std::cout << "Readout buffer size: "
				<< device.getBuffer().getBufferSize() << std::endl;

		std::ofstream file("data");
		if (!file) {
			std::cerr << "Cannot open file for write\n";
			ERREXIT();
		}

		ret = CAEN_DGTZ_SWStartAcquisition(device.getHandle());
		CHECK(ret, "starting acquisition");

		for (eventCounter = 0; eventCounter < maxEvent;) {
			if (device.hasNextEvent()) {
				eventCounter++;

				CAEN_DGTZ_EventInfo_t eventInfo;
				auto const evt = device.nextEvent(eventInfo);

				now = time(NULL);
				if (now > last) {
					verbose = 1;
					last = now;
				} else {
					verbose = 0;
				}

				if (verbose) {
					std::cout << "Event #=" << eventInfo.EventCounter
							<< ", size=" << eventInfo.EventSize << ", channels="
							<< eventInfo.ChannelMask << ", time="
							<< (eventInfo.TriggerTimeTag & ~0x80000000)
							<< std::endl;
				}

				//*************************************
				// Event Elaboration
				//*************************************

				for (j = 0; j < boardInfo.Channels; j++) {
					if ((1 << j) & eventInfo.ChannelMask) {
						uint32_t const numOfSamples = evt->ChSize[j];
						auto const begin = evt->DataChannel[j];
						auto const end = begin + numOfSamples;
						auto const minSample = std::min_element(begin, end);
						auto const maxSample = std::max_element(begin, end);

						if (numOfSamples > 0) {
							if (j == 0 && !waveformWritten) {
								// write waveform on channel #0 to a text file

								std::for_each(begin, end, [&file](uint16_t s) {
									file << s << std::endl;
								});

								waveformWritten = true;
							}
						}

						if (verbose) {
							std::cout << "Channel " << j << ", # samples="
									<< numOfSamples << ", min = " << *minSample
									<< " at " << std::distance(begin, minSample)
									<< ", max = " << *maxSample << " at "
									<< std::distance(begin, maxSample)
									<< std::endl;
						}
					}
				}
			} else {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}

		ret = CAEN_DGTZ_Reset(device.getHandle());
		CHECK(ret, "resetting digitizer");

	} catch (std::exception& e) {
		std::cerr << "exception " << e.what() << std::endl;
	}

	return 0;
}
