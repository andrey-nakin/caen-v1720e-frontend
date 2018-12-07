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
		std::string const& keyName, BOOL const create, bool const value) {

	std::int32_t v = value ? 1 : 0;
	INT bufSize = sizeof(v);
	INT const status = db_get_value(hDB, hKeyRoot, keyName.c_str(), &v,
			&bufSize, TID_BOOL, create);

	if (DB_SUCCESS != status) {
		throw midas::Exception(status,
				std::string("Error reading ODB key ") + keyName);
	}

	return v ? true : false;

}

std::string getValueString(HNDLE const hDB, HNDLE const hKeyRoot,
		std::string const& keyName, BOOL const create, std::string const& value,
		std::size_t const size) {

	std::vector<char> str(size);
	INT bufSize = size;
	::strlcpy(&str[0], value.c_str(), size);
	INT const status = db_get_value(hDB, hKeyRoot, keyName.c_str(), &str[0],
			&bufSize, TID_STRING, create);

	if (DB_SUCCESS != status) {
		throw midas::Exception(status,
				std::string("Error reading ODB key ") + keyName);
	}

	return &str[0];

}

}
