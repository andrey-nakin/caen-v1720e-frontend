#ifndef	__frontend_util_lib_types_hxx__
#define	__frontend_util_lib_types_hxx__

#include <cstdint>

namespace util {

typedef struct
	__attribute__((packed)) {

		uint32_t boardId;
		uint32_t channelMask;
		uint32_t eventCounter;
		uint32_t timeStampLo;
		uint32_t timeStampHi;

	} InfoBank;

	}

#endif	//	__frontend_util_lib_types_hxx__
