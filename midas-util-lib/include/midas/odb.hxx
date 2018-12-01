#ifndef	__midas_lib_odb_hxx__
#define	__midas_lib_odb_hxx__

#include <cstdint>
#include <string>
#include <midas.h>

namespace odb {

template<typename T, DWORD type>
T getValue(HNDLE const hDB, HNDLE const hKeyRoot, std::string const& keyName,
		BOOL const create, T value) {

	INT bufSize = sizeof(value);
	INT const status = db_get_value(hDB, hKeyRoot, keyName.c_str(), &value,
			&bufSize, type, create);

	if (DB_SUCCESS != status) {
		std::string msg("Error reading ODB key ");
		msg += keyName;
		msg += ", status %d";
		cm_msg(MERROR, frontend_name, msg.c_str(), status);

		throw std::exception();
	}

	return value;

}

bool getValueBool(HNDLE const hDB, HNDLE const hKeyRoot,
		std::string const& keyName, BOOL const create, bool const value) {

	std::int32_t v = value ? 1 : 0;
	INT bufSize = sizeof(v);
	INT const status = db_get_value(hDB, hKeyRoot, keyName.c_str(), &v,
			&bufSize, TID_BOOL, create);

	if (DB_SUCCESS != status) {
		std::string msg("Error reading ODB key ");
		msg += keyName;
		msg += ", status %d";
		cm_msg(MERROR, frontend_name, msg.c_str(), status);

		throw std::exception();
	}

	return v ? true : false;

}

std::string getValueString(HNDLE const hDB, HNDLE const hKeyRoot,
		std::string const& keyName, BOOL const create, std::string const& value,
		std::size_t const size = 256) {

	std::vector<char> str(size);
	INT bufSize = size;
	::strlcpy(&str[0], value.c_str(), size);
	INT const status = db_get_value(hDB, hKeyRoot, keyName.c_str(), &str[0],
			&bufSize, TID_STRING, create);

	if (DB_SUCCESS != status) {
		std::string msg("Error reading ODB key ");
		msg += keyName;
		msg += ", status %d";
		cm_msg(MERROR, frontend_name, msg.c_str(), status);

		throw std::exception();
	}

	return &str[0];

}

auto const getValueChar = getValue<char, TID_CHAR>;

auto const getValueInt8 = getValue<std::int8_t, TID_SBYTE>;

auto const getValueUInt8 = getValue<std::uint8_t, TID_BYTE>;

auto const getValueInt16 = getValue<std::int16_t, TID_SHORT>;

auto const getValueUInt16 = getValue<std::uint16_t, TID_WORD>;

auto const getValueInt32 = getValue<std::int32_t, TID_INT>;

auto const getValueUInt32 = getValue<std::uint32_t, TID_DWORD>;

auto const getValueFloat = getValue<float, TID_FLOAT>;

auto const getValueDouble = getValue<double, TID_DOUBLE>;

}

#endif	//	__midas_lib_odb_hxx__
