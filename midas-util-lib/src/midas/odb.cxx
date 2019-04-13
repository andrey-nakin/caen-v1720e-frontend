#include <iomanip>
#include <sstream>
#include <algorithm>
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

void setValue(HNDLE const hDB, HNDLE const hKeyRoot, std::string const& keyName,
		bool const value) {

	INT status;
	HNDLE hKey;

	status = db_find_key(hDB, hKeyRoot, keyName.c_str(), &hKey);
	if (status == DB_NO_KEY) {
		std::string msg1 = std::string("creating key ") + keyName;
		cm_msg(MERROR, "odb", msg1.c_str(), 0);

		status = db_create_key(hDB, hKeyRoot, keyName.c_str(), TID_BOOL);

		msg1 = std::string("created key ") + keyName + " " + std::to_string(status);
		cm_msg(MERROR, "odb", msg1.c_str(), 0);

		if (DB_SUCCESS != status) {
			throw midas::Exception(status,
					std::string("Error creating ODB key ") + keyName);
		}
		status = db_find_key(hDB, hKeyRoot, keyName.c_str(), &hKey);
	}
	if (DB_SUCCESS != status) {
		throw midas::Exception(status,
				std::string("Error finding ODB key ") + keyName);
	}

	int32_t const v = value ? 1 : 0;
	status = db_set_data(hDB, hKey, &v, sizeof(v), 1, TID_BOOL);

	if (DB_SUCCESS != status) {
		throw midas::Exception(status,
				std::string("Error writing ODB key ") + keyName);
	}

}

std::string getValueString(HNDLE const hDB, HNDLE const hKeyRoot,
		std::string const& keyName, std::string const& initValue,
		bool const create, std::size_t const aSize) {

	char value[ODB_MAX_STRING_LENGTH];
	value[0] = 0;
	int size = std::min(sizeof(value), aSize);
	::strlcpy(value, initValue.c_str(), size);

	INT const status = db_get_value(hDB, hKeyRoot, keyName.c_str(), &value,
			&size, TID_STRING, create ? TRUE : FALSE);

	if (DB_SUCCESS != status) {
		throw midas::Exception(status,
				std::string("Error reading ODB key ") + keyName);
	}

	return value;

}

static std::string get_value(HNDLE const hDB, HNDLE const hDir,
		std::string const& name, std::size_t const defSize) {

	char value[ODB_MAX_STRING_LENGTH];
	value[0] = 0;
	int size = defSize;
	int status = db_get_value(hDB, hDir, name.c_str(), &value, &size,
			TID_STRING, FALSE);
	if (status != DB_SUCCESS) {
		return "";
	}
	return value;

}

static void set_value(HNDLE const hDB, HNDLE const hDir,
		std::string const& name, std::string const & set,
		std::string const & def, std::size_t const size) {

	std::string const s = set + " (one of:" + def + ")";
	db_set_value(hDB, hDir, name.c_str(), s.c_str(), size, 1, TID_STRING);

}

static std::string check_add(std::string const& v, std::string const & val,
		std::string const& str, std::string& def) {

	def += std::string(" ") + str;
	if (!v.empty()) {
		return v; // keep returning the first selection
	}
	if (val.find(str) == 0) {
		return str; // if no selection yet, return the new selection
	}
	return v;

}

std::string getValueString(HNDLE const hDB, HNDLE const hKeyRoot,
		std::string const& keyName, std::vector<std::string> const& validValues,
		std::size_t const size) {

	auto const val = get_value(hDB, hKeyRoot, keyName, size);

	std::string def;
	std::string s;
	std::for_each(validValues.begin(), validValues.end(),
			[&s, val, &def](std::string const& v) {
				s = check_add(s, val, v, def);
			});
	if (s.empty()) {
		s = validValues.empty() ? "" : validValues[0];
	}
	set_value(hDB, hKeyRoot, keyName, s, def, size);
	return s;

}

void setValue(HNDLE const hDB, HNDLE const hKeyRoot, std::string const& keyName,
		std::string const& value) {

	INT status;
	HNDLE hKey;

	status = db_find_key(hDB, hKeyRoot, keyName.c_str(), &hKey);
	if (DB_SUCCESS != status) {
		throw midas::Exception(status,
				std::string("Error finding ODB key ") + keyName);
	}

	status = db_set_data(hDB, hKey, value.c_str(), value.size() + 1, 1,
			TID_STRING);

	if (DB_SUCCESS != status) {
		throw midas::Exception(status,
				std::string("Error writing ODB key ") + keyName);
	}

}

std::vector<bool> getValueBoolV(HNDLE const hDB, HNDLE const hKeyRoot,
		std::string const& keyName, std::size_t const numValues,
		bool const defValue, bool const create) {

	std::vector<int32_t> v(numValues);
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
