#pragma once

#include <string>
#include <unordered_map>


/*	A property is defined by a name and a value.
	Appears as something like "pi=3.14" in an ini file.
*/
struct bsIniProperty
{
	bsIniProperty()
	{}
	~bsIniProperty()
	{}
	bsIniProperty(bsIniProperty&& other)
		: name(std::move(other.name))
		, value(std::move(other.value))
	{
	}

	/*	Returns true if this property is not invalid (contains no proper data).
		A property is required to contain both a name and a value in order to be
		considered good.
	*/
	inline bool good() const
	{
		return !name.empty() && !value.empty();
	}
	
	/*	Converts this property's value as converted to the template argument type.
		Returns true on successful conversions, false otherwise. If conversion is not
		successful, result is undefined.
	*/
	template <typename T>
	inline bool tryGetValue(T& result) const;

	/*	Calls tryGetValue, but does not return whether the conversion was
		successful or failed.
		An assert will fire on failed conversions, so only use this function when it is
		known that the value can be converted successfully.
		On failure, the return value is undefined.
	*/
	template <typename T>
	inline T getValue() const;


	std::string name;
	std::string value;
};


/*	A section contains properties. Allows searching for properties by name.
	Appears as "[section name]" in an ini file.
*/
class bsIniSection
{
public:
	bsIniSection()
	{}
	~bsIniSection()
	{}
	bsIniSection(bsIniSection&& other)
		: mName(std::move(other.mName))
		, mProperties(std::move(other.mProperties))
	{}

	/*	True if this section contains no properties. Note that this means that sections
		with no properties or only commented out properties will be considered empty
	*/
	inline bool empty() const
	{
		return mProperties.empty();
	}

	//Reset this section.
	inline void clear()
	{
		mProperties.clear();
		mName.clear();
	}

	/*	Searches for a property with the given name.
		Returns the property if found, or an empty property if not found
		Use bsIniProperty::good() to see if the return value was found.

		Possibly useful functions: stod, stof, stoi, stol, stold, stoll, stoul, stoull.
	*/
	inline const bsIniProperty* getProperty(const std::string& propertyName) const;

	//Gets the name of this section.
	inline const std::string& getName() const
	{
		return mName;
	}

	//Sets the name of this section.
	inline void setName(const std::string& newName)
	{
		mName = newName;
	}

	//Returns internal set used to contain owned properties with read-only access.
	inline const std::unordered_map<std::string, bsIniProperty>& getProperties() const
	{
		return mProperties;
	}

	//Returns internal set used to contain owned properties with read/write access.
	inline std::unordered_map<std::string, bsIniProperty>& getProperties()
	{
		return mProperties;
	}


private:
	//Name of this section.
	std::string mName;

	//Properties contained in this section.
	std::unordered_map<std::string, bsIniProperty> mProperties;
};

#include "bsIni.inl"
