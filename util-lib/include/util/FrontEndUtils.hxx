#ifndef	UTIL_LIB_FrontEndUtils_hxx
#define	UTIL_LIB_FrontEndUtils_hxx

#include <midas.h>

namespace util {

class FrontEndUtils {
public:

	static std::string equipmentName(const char* feBaseName);

	static std::string settingsKeyName(const char* equipName,
			const char* propName = nullptr);

	static HNDLE settingsKey(const char* equipName);

private:

	FrontEndUtils() = delete;

};

}

#endif	//	UTIL_LIB_FrontEndUtils_hxx
