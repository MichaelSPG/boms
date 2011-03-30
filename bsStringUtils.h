#ifndef BSSTRINGUTILS_H
#define BSSTRINGUTILS_H

#include <string>
#include <sstream>
#include <algorithm>

#include <Windows.h>


class bsStringUtils
{
public:
	/*
	inline static std::wstring toWString(const std::string& str)
	{
		std::wstring w(str.size(), L'\0');
		std::copy(str.begin(), str.end(), w);
		return w;
	}
	*/
	inline static std::wstring utf8ToUtf16(const std::string& utf8String)
	{
		std::wstring w;
		w.resize(utf8String.size());
		MultiByteToWideChar(
			CP_UTF8,            // convert from UTF-8
			0,                  // default flags
			utf8String.data(),        // source UTF-8 string
			utf8String.length(),      // length (in chars) of source UTF-8 string
			&w[0],          // destination buffer
			w.length()      // size of destination buffer, in wchar_t's
			);
		return w;
	}

	//Converts any type std::stringstream supports to a string.
	template<typename T>
	inline static std::string toString(const T& t)
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
	inline static std::string toStringWithPrecision(const T& t, const unsigned int precision)
	{
		std::stringstream ss;
		ss.setf(std::ios::floatfield, std::ios::fixed);
		ss.precision(precision);
		ss << t;
		return ss.str();
	}
};

#endif // BSSTRINGUTILS_H