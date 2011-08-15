#include "bsIni.h"

#include <algorithm>
#include <string>
#include <sstream>


/*	Tries to convert a string to a type with the provided conversion function,
	catching any conversion failure exceptions.
	Returns true on successful conversion, false otherwise. If returning false, the value
	of the output parameter is undefined.
*/
template <typename T, typename ConversionFunction>
inline bool tryConvertValue(const std::string& value, ConversionFunction func, T& output)
{
	T conversionResult;

	try
	{
		conversionResult = func(value, nullptr);
	}
	catch (const std::invalid_argument&)
	{
		return false;
	}
	catch (const std::out_of_range&)
	{
		return false;
	}

	output = conversionResult;
	return true;
}

//Same as above, but also supports specifying base for integer conversion.
template <typename T, typename ConversionFunction>
inline bool tryConvertValue(const std::string& value, ConversionFunction func, T& output,
	int base)
{
	T conversionResult;

	try
	{
		conversionResult = func(value, nullptr, base);
	}
	catch (const std::invalid_argument&)
	{
		return false;
	}
	catch (const std::out_of_range&)
	{
		return false;
	}

	output = conversionResult;
	return true;
}

//Template specialization for bool.
template <>
inline bool bsIniProperty::tryGetValue(bool& result) const
{
	//Convert the value to lower case for easier comparison.
	std::string lowerCaseValue(value);
	std::transform(lowerCaseValue.begin(), lowerCaseValue.end(),
		lowerCaseValue.begin(), tolower);

	if (lowerCaseValue == "true" || lowerCaseValue == "1")
	{
		result = true;
		return true;
	}
	else if (lowerCaseValue == "false" || lowerCaseValue == "0")
	{
		result = false;
		return true;
	}
	else
	{
		return false;
	}
}

/*	Template specialization for float.
	The following number specializations use the tryConvertValue function with standard
	library conversion functions.
*/
template <>
inline bool bsIniProperty::tryGetValue(float& result) const
{
	//stof is overloaded with std::string and std::wstring functions, need to specify
	//which version to call.
	return tryConvertValue(value,
		(float (*)(const std::string&, size_t*))std::stof, result);
}

//Template specialization for double.
template <>
inline bool bsIniProperty::tryGetValue(double& result) const
{
	return tryConvertValue(value,
		(double (*)(const std::string&, size_t*))std::stod, result);
}

//Template specialization for long double.
template <>
inline bool bsIniProperty::tryGetValue(long double& result) const
{
	return tryConvertValue(value,
		(long double (*)(const std::string&, size_t*))std::stold, result);
}

//Template specialization for int.
template <>
inline bool bsIniProperty::tryGetValue(int& result) const
{
	return tryConvertValue(value,
		(int (*)(const std::string&, size_t*, int))std::stoi, result, 10);
}

//Template specialization for long.
template <>
inline bool bsIniProperty::tryGetValue(long& result) const
{
	return tryConvertValue(value,
		(long (*)(const std::string&, size_t*, int))std::stol, result, 10);
}

//Template specialization for long long.
template <>
inline bool bsIniProperty::tryGetValue(long long& result) const
{
	return tryConvertValue(value,
		(long long (*)(const std::string&, size_t*, int))std::stoll, result, 10);
}

//Template specialization for unsigned long.
template <>
inline bool bsIniProperty::tryGetValue(unsigned long& result) const
{
	return tryConvertValue(value,
		(unsigned long (*)(const std::string&, size_t*, int))std::stoul, result, 10);
}

//Template specialization for unsigned long long.
template <>
inline bool bsIniProperty::tryGetValue(unsigned long long& result) const
{
	return tryConvertValue(value,
		(unsigned long long (*)(const std::string&, size_t*, int))std::stoull, result, 10);
}

//Unknown type, use a stringstream.
template <typename T>
inline bool bsIniProperty::tryGetValue(T& result) const
{
	std::stringstream ss(value);
	T temp;
	ss >> temp;
	result = temp;
	const bool fail = ss.fail();
	return !fail;
}


template <typename T>
T bsIniProperty::getValue() const
{
	T conversionResult;
	const bool conversionSucceeded = tryGetValue(conversionResult);
	assert(conversionSucceeded && "Conversion failed.");

	return conversionResult;
}

const bsIniProperty* bsIniSection::getProperty(const std::string& propertyName) const
{
	//Make a dummy property to search for it.
	auto findResult = mProperties.find(propertyName);

	if (findResult == mProperties.end())
	{
		//Not found.
		return nullptr;
	}

	return &findResult->second;
}