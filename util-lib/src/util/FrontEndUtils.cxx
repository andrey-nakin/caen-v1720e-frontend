#include <iomanip>
#include <sstream>
#include <midas/odb.hxx>
#include "util/FrontEndUtils.hxx"

#ifndef NEED_NO_EXTERN_C
extern "C" {
#endif

extern HNDLE hDB;

INT get_frontend_index();

#ifndef NEED_NO_EXTERN_C
}
#endif

namespace util {

std::string FrontEndUtils::settingsKeyName(const char* const feBaseName,
		const char* const propName) {

	std::stringstream s;
	s << "/Equipment/" << feBaseName;
	if (get_frontend_index() >= 0) {
		s << std::setfill('0') << std::setw(2) << get_frontend_index();
	}
	s << "/Settings";
	if (propName) {
		s << "/" << propName;
	}
	return s.str();

}

HNDLE FrontEndUtils::settingsKey(const char* feBbaseName) {

	return odb::findKey(hDB, 0, settingsKeyName(feBbaseName));

}

}
