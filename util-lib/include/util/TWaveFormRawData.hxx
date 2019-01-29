#ifndef	UTIL_LIB_TWaveFormRawData_hxx
#define	UTIL_LIB_TWaveFormRawData_hxx

#include <cstdint>
#include <TGenericData.hxx>
#include "types.hxx"

namespace util {

class TWaveFormRawData: public TGenericData {
public:

	typedef uint16_t value_type;
	typedef value_type const* const_iterator_type;
	using difference_type = typename std::iterator_traits<const_iterator_type>::difference_type;

	TWaveFormRawData(int bklen, int bktype, const char* name, void *pdata);

	static const char* bankName(uint8_t channelNo);

	std::size_t numOfSamples() const {

		return GetSize();

	}

	const_iterator_type begin() const {

		return GetData16();

	}

	const_iterator_type end() const {

		return begin() + numOfSamples();

	}

};

}

#endif	//	UTIL_LIB_TWaveFormRawData_hxx
