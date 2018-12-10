#ifndef	__frontend_util_lib_types_hxx__
#define	__frontend_util_lib_types_hxx__

#include <cstdint>

namespace util {

typedef struct
	__attribute__((packed)) {

		uint32_t boardId;
		uint32_t channelMask;
		uint32_t eventCounter;
		uint64_t timeStamp;

	} InfoBank;

	}

#endif	//	__frontend_util_lib_types_hxx__
