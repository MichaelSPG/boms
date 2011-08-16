#include "bsIniParser.h"

#include <sstream>

#include "bsAssert.h"


void bsIniParser::parseData(const char* data, size_t dataSizeBytes)
{
	BS_ASSERT(data);
	BS_ASSERT(dataSizeBytes > 0);

	//Don't want old data from previous parses mixed in with this parse.
	mSections.clear();

	//Assume line length, save some reallocation, possibly waste some space
	std::vector<std::string> lines;
	lines.reserve(dataSizeBytes / 30);

	//Split into individual lines.
	std::stringstream ss(std::move(std::string(data, data + dataSizeBytes)));
	std::string line;
	while (std::getline(ss, line))
	{
		lines.push_back(line);
	}

	parseLines(lines);
}

inline bool bsIniParser::isBadLine(const std::string& line) const
{
	//False if line is empty or if it starts with comment or whitespace.
	if (line.empty())
	{
		return true;
	}
	const char first = line[0];
	return first == ';' || first == ' ' || first == '\t';
}

void bsIniParser::parseLines(const std::vector<std::string>& lines)
{
	//The section which properties are currently being written to.
	bsIniSection currentSection;

	for (size_t i = 0, count = lines.size(); i < count; ++i)
	{
		const std::string& currentLine = lines[i];

		if (isBadLine(currentLine))
		{
			continue;
		}

		if (currentLine[0] == '[')
		{
			//Start of new section.

			//bsIniSection name ("[something]")
			std::string newSectionName(std::move(parseSectionStart(currentLine)));

			if (!newSectionName.empty())
			{
				if (!currentSection.empty())
				{
					//Insert previous section
					
					//Duplicate section names is not supported.
					//assert(mSections.find(currentSection.getName()) == mSections.end()
					//	&& "Duplicate section names found!");

					mSections.insert(std::move(std::make_pair<std::string, bsIniSection>
						(currentSection.getName(), currentSection)));
					currentSection.clear();
				}

				currentSection.clear();
				currentSection.setName(newSectionName);
			}
		}
		else
		{
			//Normal prop line ("x=y")

			bsIniProperty prop = parseProperty(currentLine);
			if (prop.good())
			{
				//Duplicate property names is not supported.
				//assert(currentSection.getProperties().find(prop.name) ==
				//	currentSection.getProperties().end()
				//	&& "Duplicate property names found!");

				currentSection.getProperties().insert(std::move(
					std::make_pair<std::string, bsIniProperty>(prop.name, prop)));
			}
		}
	}

	/*	Check if anything has been parsed at all, and if so, insert the parsed section.
		This will also insert the last parsed section, as a section is normally only
		inserted when a new section starts.
	*/
	if (!currentSection.empty())
	{
		//Duplicate section names is not supported.
		//assert(mSections.find(currentSection.getName()) == mSections.end()
		//	&& "Duplicate section names found!");

		mSections.insert(std::move(std::make_pair<std::string, bsIniSection>
			(currentSection.getName(), currentSection)));
	}
}

std::string bsIniParser::parseSectionStart(const std::string& line)
{
	//Get index of end bracket, then get substring between the two.
	//"[xyz]" becomes "xyz".
	
	const size_t endBracketIndex = line.find_first_of(']', 1);
	if (endBracketIndex != std::string::npos)
	{
		return(line.substr(1, endBracketIndex - 1));
	}
	else
	{
		return std::string();
	}
}

bsIniProperty bsIniParser::parseProperty(const std::string& line)
{
	const size_t findIndex = line.find_first_of('=');
	if (findIndex != std::string::npos)
	{
		//Insert the string into a property so that name = "x" and value = "y"
		//for the line "x=y".
		bsIniProperty prop;
		prop.name = line.substr(0, findIndex);
		prop.value = line.substr(findIndex + 1);


		//Trim whitespace.

		//Trim end for name. If there was whitespace before the name, this function
		//wouldn't have been called
		while (!prop.name.empty() && prop.name.back() == ' ' || prop.name.back() == '\t')
		{
			prop.name.pop_back();
		}

		//Trim end for value.
		while (!prop.value.empty() && (prop.value.back() == ' ' || prop.value.back() == '\t'))
		{
			prop.value.pop_back();
		}
		//Get index of first non-whitespace char.
		const size_t firstAfterWhitespaceIndex = prop.value.find_first_not_of(" \t");
		if (firstAfterWhitespaceIndex != 0 && firstAfterWhitespaceIndex != std::string::npos)
		{
			//First non-whitespace char was not first char, trim beginning.
			prop.value.erase(prop.value.begin(), prop.value.begin() + firstAfterWhitespaceIndex);
		}

		return prop;
	}
	else
	{
		//No '=' found, invalid property.
		return bsIniProperty();
	}
}

const bsIniSection* bsIniParser::getSection(const std::string& sectionName) const
{
	const auto findResult = mSections.find(sectionName);
	if (findResult != mSections.end())
	{
		//Found it, return its address.
		return &findResult->second;
	}

	//Didn't find it.
	return nullptr;
}

const bsIniProperty* bsIniParser::getProperty(const std::string& propertyName) const
{
	//Iterate through all sections until we find one which contains the requested property.
	for (auto itr = mSections.cbegin(), end = mSections.cend(); itr != end; ++itr)
	{
		const bsIniProperty* findResult = itr->second.getProperty(propertyName);
		if (findResult != nullptr)
		{
			return findResult;
		}
	}

	//Didn't find it.
	return nullptr;
}
