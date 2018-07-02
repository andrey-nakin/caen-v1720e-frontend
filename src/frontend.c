#include <stdint.h>
#include <stdio.h>
#include <CAENDigitizer.h>

int const linkNum = 0;
int const conetNode = 0;
uint32_t const vmeBaseAddr = 0;

int main(int argc, char* argv[]) {
	CAEN_DGTZ_ErrorCode ret;
	int handle;

	ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, linkNum, conetNode,
			vmeBaseAddr, &handle);
	if (ret != 0) {
		printf("Error %d opening digitizer\n", (int) ret);
		return 1;
	}

	CAEN_DGTZ_CloseDigitizer(handle);

	return 0;
}
