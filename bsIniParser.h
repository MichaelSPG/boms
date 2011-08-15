#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "bsIni.h"


/*	Parser for ini files.
	http://en.wikipedia.org/wiki/INI_file
	
	Multiple identically named properties in a single section or multiple identically
	named sections results in undefined behavior.
	Comments must start at the first character of a line, and continues until the end
	of that line.
	Whitespace is allowed anywhere. Lines starting with whitespace are ignored.


	Example ini file contents (ignore the first tab of each line):
	property1=1.23

	[bsIniSection name]
	size=93.4
	color=gre\n   \t\t\ten
	;the \n and \t are ignored in the above line (stored as the string "\t", not char '\t').
	text	=;this is not a; comment
	; this is a comment
	[another section]

	pi =     4
*/
class bsIniParser
{
public:
	bsIniParser()
	{}

	~bsIniParser()
	{}

	/*	Parses raw text as an ini file.
		Use the below get functions to retrieve information parsed after calling this
		function.

		It is OK to call this function multiple times, but results will not be merged,
		old results will be cleared before starting a new parse.
	*/
	void parseData(const char* data);


	/*	Returns the section with the specified name.
		If the section was not found, null is returned.
		Unnamed sections can get found by using "" as input.

		Modifying the parser's internal data after getting a pointer to a section
		results in undefined behavior.
	*/

	const bsIniSection* getSection(const std::string& sectionName) const;


	/*	Gets the specified property from an undefined section.
		This can be useful if you know the parsed text only contains none or a single
		section, or if you don't know in which section a property exists.
		If you do know which section the property exists in, it is faster to get that
		section and find the property manually.

		If the property was not found, null is returned.
	*/
	const bsIniProperty* getProperty(const std::string& propertyName) const;


private:
	//Returns true for lines which should not be parsed (malformed or commented out most likely).
	inline bool isBadLine(const std::string& line) const;

	//Iterates through input lines and puts valid lines into their respective sections.
	void parseLines(const std::vector<std::string>& lines);

	//Parse line starting with '['
	std::string parseSectionStart(const std::string& line);

	//Parse normal line, should contain x=y
	bsIniProperty parseProperty(const std::string& line);


	//The sections contained in a loaded ini file.
	std::unordered_map<std::string, bsIniSection>	mSections;
};
