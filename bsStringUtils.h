#pragma once


#include <string>
#include <sstream>
#include <algorithm>

#include <Windows.h>


/*	Contains various string manipulation and generation functions.	
*/
namespace bsStringUtils
{
//Converts UTF-8 strings to UTF-16 strings.
inline std::wstring utf8ToUtf16(const std::string& utf8String)
{
	const unsigned int stringLength = utf8String.length();

	std::wstring utf16String;
	utf16String.resize(stringLength);

	//MultiByteToWideChar fails with zero-length strings
	if (stringLength == 0)
	{
		return utf16String;
	}

	MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), stringLength, &utf16String[0], stringLength);

	return utf16String;
}

//Converts any type std::stringstream supports to a string.
template<typename T>
inline std::string toString(const T& t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}

/*	Converts any type std::stringstream supports to a string with fixed precision.
	For instance, 1 can get converted as 1.000 with precision set to 3,
	1.748923849032 can get converted to 1.7 with precision set to 1, and
	10.0 can get converted to 10 with precision set to 0.
*/
template<typename T>
inline std::string toStringWithPrecision(const T& t, const unsigned int precision)
{
	std::stringstream ss;
	ss.setf(std::ios::floatfield, std::ios::fixed);
	ss.precision(precision);
	ss << t;
	return ss.str();
}

/*	Converts a number representing a number of bytes to a string.

	The number 1024 will be converted to "1.0 KB", 590558003 to "563.2 MB", 3 to "3 bytes",
	and so on. Units other than bytes are representet with numbers having 1 decimal place.
	The maximum unit is GB. Values over 1 GB are always specified in GB.

	The parameter 'buffer' needs to be able to fit the entire converted string.
	11 bytes is enough for anything up to 999.9 GB.
*/
template <typename SizeType, size_t Size>
void byteToString(SizeType bytes, char (&buffer)[Size])
{
	static_assert(std::is_integral<SizeType>::value, "SizeType must be an integer");
	static_assert(Size >= 11, "Buffer is too small, must be able to hold at least 11 characters");

	if (bytes >= 1024 * 1024 * 1024)// >= 1 GB
	{
		sprintf(buffer, "%.1f GB", bytes / (1024.0 * 1024.0 * 1024.0));
	}
	else if (bytes >= 1024 * 1024)// >= 1 MB
	{
		sprintf(buffer, "%.1f MB", bytes / (1024.0 * 1024.0));
	}
	else if (bytes >= 1024)// >= 1 KB
	{
		sprintf(buffer, "%.1f KB", bytes / 1024.0);
	}
	else// < 1 KB
	{
		sprintf(buffer, "%u bytes", bytes);
	}
}

template <typename SizeType, size_t Size>
void byteToStringWide(SizeType bytes,  wchar_t(&buffer)[Size])
{
	static_assert(std::is_integral<SizeType>::value, "SizeType must be an integer");
	static_assert(Size >= 11, "Buffer is too small, must be able to hold at least 11 characters");

	if (bytes >= 1024 * 1024 * 1024)// >= 1 GB
	{
		swprintf(buffer, Size, L"%.1f GB", bytes / (1024.0 * 1024.0 * 1024.0));
	}
	else if (bytes >= 1024 * 1024)// >= 1 MB
	{
		swprintf(buffer, Size, L"%.1f MB", bytes / (1024.0 * 1024.0));
	}
	else if (bytes >= 1024)// >= 1 KB
	{
		swprintf(buffer, Size, L"%.1f KB", bytes / 1024.0);
	}
	else// < 1 KB
	{
		swprintf(buffer, Size, L"%u bytes", bytes);
	}
}
};
