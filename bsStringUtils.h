#ifndef BS_STRINGUTILS_H
#define BS_STRINGUTILS_H

#include "bsConfig.h"

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
};

#endif // BS_STRINGUTILS_H
