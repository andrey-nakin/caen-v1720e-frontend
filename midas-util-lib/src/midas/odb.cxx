#include <iomanip>
#include <sstream>
#include <midas/odb.hxx>

namespace odb {

HNDLE findKey(HNDLE const hDB, HNDLE const hKey, std::string const& keyName) {

	HNDLE hResult;
	INT const status = db_find_key(hDB, hKey, keyName.c_str(), &hResult);
	if (DB_SUCCESS != status) {
		throw midas::Exception(status,
				std::string("Error finding ODB key ") + keyName);
	}
	return hResult;

}

bool getValueBool(HNDLE const hDB, HNDLE const hKeyRoot,
		std::string const& keyName, bool const value, bool const create) {

	std::int32_t v = value ? 1 : 0;
	INT bufSize = sizeof(v);
	INT const status = db_get_value(hDB, hKeyRoot, keyName.c_str(), &v,
			&bufSize, TID_BOOL, create ? TRUE : FALSE);

	if (DB_SUCCESS != status) {
		throw midas::Exception(status,
				std::string("Error reading ODB key ") + keyName);
	}

	return v ? true : false;

}

std::string getValueString(HNDLE const hDB, HNDLE const hKeyRoot,
		std::string const& keyName, std::string const& value, bool const create,
		std::size_t const size) {

	std::vector<char> str(size);
	INT bufSize = size;
	::strlcpy(&str[0], value.c_str(), size);
	INT const status = db_get_value(hDB, hKeyRoot, keyName.c_str(), &str[0],
			&bufSize, TID_STRING, create ? TRUE : FALSE);

	if (DB_SUCCESS != status) {
		throw midas::Exception(status,
				std::string("Error reading ODB key ") + keyName);
	}

	return &str[0];

}

std::vector<bool> getValueBoolV(HNDLE const hDB, HNDLE const hKeyRoot,
		std::string const& keyName, std::size_t const numValues,
		bool const defValue, bool const create) {

	std::vector < int32_t > v(numValues);
	for (std::size_t i = 0; i < numValues; i++) {
		v[i] = defValue;
	}
	INT bufSize = sizeof(v[0]) * numValues;
	INT const status = db_get_value(hDB, hKeyRoot, keyName.c_str(), &v[0],
			&bufSize, TID_BOOL, create ? TRUE : FALSE);

	if (DB_SUCCESS != status) {
		throw midas::Exception(status,
				std::string("Error reading ODB key ") + keyName);
	}

	std::vector<bool> result(numValues);
	for (std::size_t i = 0; i < numValues; i++) {
		result[i] = v[i] ? true : false;
	}
	return result;

}

std::string equipSettingsKeyName(const char* const equipName,
		const char* const propName) {

	std::stringstream s;
	s << "/Equipment/" << equipName << "/Settings";
	if (propName) {
		s << "/" << propName;
	}
	return s.str();

}

std::string equipSettingsKeyName(std::string const& equipName,
		INT const feIndex, const char* const propName) {

	std::stringstream s;
	s << "/Equipment/" << equipName;
	if (feIndex >= 0) {
		s << std::setfill('0') << std::setw(2) << feIndex;
	}
	s << "/Settings";
	if (propName) {
		s << "/" << propName;
	}
	return s.str();

}

}
