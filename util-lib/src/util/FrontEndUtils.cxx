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

std::string FrontEndUtils::equipmentName(const char* const feBaseName) {

	std::stringstream s;
	s << feBaseName;
	if (get_frontend_index() >= 0) {
		s << std::setfill('0') << std::setw(2) << get_frontend_index();
	}
	return s.str();

}

std::string FrontEndUtils::settingsKeyName(const char* const equipName,
		const char* const propName) {

	std::stringstream s;
	s << "/Equipment/" << equipName << "/Settings";
	if (propName) {
		s << "/" << propName;
	}
	return s.str();

}

HNDLE FrontEndUtils::settingsKey(const char* equipName) {

	return odb::findKey(hDB, 0, settingsKeyName(equipName));

}

}
