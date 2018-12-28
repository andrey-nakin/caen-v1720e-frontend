#include <midas/odb.hxx>
#include "util/FrontEndUtils.hxx"

#ifndef NEED_NO_EXTERN_C
extern "C" {
#endif

extern HNDLE hDB;
extern const char *frontend_name;

#ifndef NEED_NO_EXTERN_C
}
#endif

namespace util {

std::string FrontEndUtils::settingsKeyName(const char* const equipName,
		const char* const propName) {

	return odb::equipSettingsKeyName(equipName, propName);

}

std::string FrontEndUtils::settingsKeyName(std::string const& equipName,
		INT const feIndex, const char* const propName) {

	return odb::equipSettingsKeyName(equipName, feIndex, propName);

}

HNDLE FrontEndUtils::settingsKey(const char* equipName) {

	return odb::findKey(hDB, 0, settingsKeyName(equipName));

}

HNDLE FrontEndUtils::settingsKey(std::string const& equipName,
		INT const feIndex) {

	return odb::findKey(hDB, 0, settingsKeyName(equipName, feIndex));

}

INT FrontEndUtils::handleCaenException(caen::Exception const& ex) {

	INT const status = CM_SET_ERROR;
	cm_msg(MERROR, frontend_name, ex.what(), status);
	return status;

}
}
