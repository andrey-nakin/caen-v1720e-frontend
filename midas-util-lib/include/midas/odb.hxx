#ifndef	__midas_util_lib_odb_hxx__
#define	__midas_util_lib_odb_hxx__

#include <cstdint>
#include <string>
#include <vector>
#include <midas.h>
#include "midas/exception.hxx"

namespace odb {

constexpr std::size_t ODB_MAX_STRING_LENGTH = 256;

template<typename T, DWORD type>
T getValue(HNDLE const hDB, HNDLE const hKeyRoot, std::string const& keyName,
		T value, bool const create) {

	INT bufSize = sizeof(value);
	INT const status = db_get_value(hDB, hKeyRoot, keyName.c_str(), &value,
			&bufSize, type, create ? TRUE : FALSE);

	if (DB_SUCCESS != status) {
		throw midas::Exception(status,
				std::string("Error reading ODB key ") + keyName);
	}

	return value;

}

template<typename T, DWORD type>
std::vector<T> getValueV(HNDLE const hDB, HNDLE const hKeyRoot,
		std::string const& keyName, std::size_t const numValues,
		T const defValue, bool const create) {

	std::vector < T > value(numValues, defValue);
	if (numValues > 0) {
		INT bufSize = sizeof(T) * numValues;
		INT const status = db_get_value(hDB, hKeyRoot, keyName.c_str(),
				&value[0], &bufSize, type, create ? TRUE : FALSE);

		if (DB_SUCCESS != status) {
			throw midas::Exception(status,
					std::string("Error reading ODB key ") + keyName);
		}
	}

	return value;

}

bool getValueBool(HNDLE const hDB, HNDLE const hKeyRoot,
		std::string const& keyName, bool const value, bool const create);

std::string getValueString(HNDLE const hDB, HNDLE const hKeyRoot,
		std::string const& keyName, std::string const& value, bool const create,
		std::size_t const size = ODB_MAX_STRING_LENGTH);

std::vector<bool> getValueBoolV(HNDLE hDB, HNDLE hKeyRoot,
		std::string const& keyName, std::size_t numValues, bool defValue,
		bool create);

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

auto const getValueCharV = getValueV<char, TID_CHAR>;

auto const getValueInt8V = getValueV<std::int8_t, TID_SBYTE>;

auto const getValueUInt8V = getValueV<std::uint8_t, TID_BYTE>;

auto const getValueInt16V = getValueV<std::int16_t, TID_SHORT>;

auto const getValueUInt16V = getValueV<std::uint16_t, TID_WORD>;

auto const getValueInt32V = getValueV<std::int32_t, TID_INT>;

auto const getValueUInt32V = getValueV<std::uint32_t, TID_DWORD>;

auto const getValueFloatV = getValueV<float, TID_FLOAT>;

auto const getValueDoubleV = getValueV<double, TID_DOUBLE>;

std::string equipSettingsKeyName(const char* equipName, const char* propName =
		nullptr);

std::string equipSettingsKeyName(std::string const& equipName, INT feIndex,
		const char* propName = nullptr);

}

#endif	//	__midas_util_lib_odb_hxx__
