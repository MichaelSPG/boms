#ifndef TO_STRING_H
#define TO_STRING_H

#include <sstream>

#define STR ToString::toString


/**	Converts various types to strings.
*/
class ToString
{
public:
	///Signed int
	static inline std::string toString(int x)
	{
		std::string s;
		std::stringstream out;
		out << x;
		s = out.str();
		return s;
	}

	///Unsigned int
	static inline std::string toString(unsigned int x)
	{
		std::string s;
		std::stringstream out;
		out << x;
		s = out.str();
		return s;
	}

	///Float
	static inline std::string toString(float x)
	{
		std::string s;
		std::stringstream out;
		out << x;
		s = out.str();
		return s;
	}

	///Unsigned long
	static inline std::string toString(unsigned long x)
	{
		std::string s;
		std::stringstream out;
		out << x;
		s = out.str();
		return s;
	}
};

#endif