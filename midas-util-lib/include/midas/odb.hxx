#ifndef	__midas_util_lib_odb_hxx__
#define	__midas_util_lib_odb_hxx__

#include <cstdint>
#include <string>
#include <midas.h>
#include "midas/exception.hxx"

namespace odb {

template<typename T, DWORD type>
T getValue(HNDLE const hDB, HNDLE const hKeyRoot, std::string const& keyName,
		BOOL const create, T value) {

	INT bufSize = sizeof(value);
	INT const status = db_get_value(hDB, hKeyRoot, keyName.c_str(), &value,
			&bufSize, type, create);

	if (DB_SUCCESS != status) {
		throw midas::Exception(status,
				std::string("Error reading ODB key ") + keyName);
	}

	return value;

}

bool getValueBool(HNDLE const hDB, HNDLE const hKeyRoot,
		std::string const& keyName, BOOL const create, bool const value);

std::string getValueString(HNDLE const hDB, HNDLE const hKeyRoot,
		std::string const& keyName, BOOL const create, std::string const& value,
		std::size_t const size = 256);

HNDLE findKey(HNDLE const hDB, HNDLE const hKey, std::string const& keyName);

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

#endif	//	__midas_util_lib_odb_hxx__
