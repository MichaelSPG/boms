#pragma once

#include "bsAssert.h"
#include <string.h>
#include <stdarg.h>


/*	String class with fixed size.

	This class will never allocate memory other than on the stack.

	The last character in the string is always a null terminator, so the max length of
	the string (excluding the null terminator) is Size - 1.
*/
template <size_t Size>
class bsFixedSizeString
{
	static_assert(sizeof(char) == 1, "This class does not work with multi byte chars");
	static_assert(Size != 0, "String size cannot be zero");

public:
	static const size_t npos = ~(size_t)0;


	bsFixedSizeString()
	{
		setLength(0);
	}

	bsFixedSizeString(const char* str)
	{
		//+1 to include null terminator.
		const size_t paramLength = strlen(str) + 1;
		BS_ASSERT2(paramLength <= Size, "Buffer overflow");

		memcpy(mData, str, paramLength);

		setLength(paramLength - 1);
	}

	bsFixedSizeString(const char* str, size_t stringLength)
	{
		//+1 to include null terminator.
		BS_ASSERT2(stringLength + 1 <= Size, "Buffer overflow");

		memcpy(mData, str, stringLength);

		setLength(stringLength);
	}

	bsFixedSizeString(const bsFixedSizeString& other)
		: mStringLength(other.mStringLength)
	{
		//+1 to copy null terminator.
		memcpy(mData, other.mData, other.mStringLength + 1);
	}


	/*	Sets a copy of str as the new content of this string.
	*/
	bsFixedSizeString& operator=(const char* str)
	{
		//+1 to include null terminator.
		const size_t paramLength = strlen(str) + 1;
		BS_ASSERT2(paramLength <= Size, "Buffer overflow");

		memmove(mData, str, paramLength);

		setLength(paramLength - 1);

		return *this;
	}

	/*	Sets a copy of other as the new content of this string.
	*/
	bsFixedSizeString& operator=(const bsFixedSizeString& other)
	{
		assign(other.c_str(), other.length());

		return *this;
	}

	/*	Assigns content to this string.
		This overwrites the current content in this string.
	*/
	void assign(const char* str, size_t stringLength)
	{
		clear();

		//+1 to include null terminator.
		BS_ASSERT2(stringLength + 1 <= Size, "Buffer overflow");

		memmove(mData, str, stringLength);

		setLength(stringLength);
	}

	/*	Returns true if this string and other are identical.
	*/
	bool operator==(const bsFixedSizeString& other) const
	{
		//Early out if lengths don't match.
		if (length() != other.length())
		{
			return false;
		}
		return compare(other) == 0;
	}

	/*	Returns true if this string and other are identical.
	*/
	bool operator==(const char* other) const
	{
		return compare(other) == 0;
	}

	/*	Compares this string with other.
		Returns zero if the two are equal, a positive value if this string is greater than
		other (by comparing ASCII values, not lexicographically), or a negative value
		if this string is less than other.
	*/
	int compare(const bsFixedSizeString& other) const
	{
		return compare(other.c_str());
	}

	/*	Compares this string with other.
		Returns zero if the two are equal, a positive value if this string is greater than
		other (by comparing ASCII values, not lexicographically), or a negative value
		if this string is less than other.
	*/
	int compare(const char* other) const
	{
		return strcmp(c_str(), other);
	}


	/*	Read-write access to indiviual character.
	*/
	char& operator[](size_t index)
	{
		BS_ASSERT2(!empty(), "Index out of range (empty string)");
		BS_ASSERT2(index <= mStringLength, "Index out of range");

		return mData[index];
	}

	/*	Read access to invidual character.
	*/
	const char& operator[](size_t index) const
	{
		BS_ASSERT2(!empty(), "Index out of range (empty string)");
		BS_ASSERT2(index <= mStringLength, "Index out of range");

		return mData[index];
	}

	/*	Returns read-write access to the last character in this string (excluding null
		terminator).
	*/
	char& back()
	{
		BS_ASSERT2(!empty(), "Index out of range (empty string)");

		//-1 to account for null terminator.
		return (*this)[mStringLength - 1];
	}

	/*	Returns the last character in this string (excluding null terminator).
	*/
	const char& back() const
	{
		BS_ASSERT2(!empty(), "Index out of range (empty string)");

		//-1 to account for null terminator.
		return (*this)[mStringLength - 1];
	}

	/*	Returns read-write access to the first character in this string.
	*/
	char& front()
	{
		BS_ASSERT2(!empty(), "Index out of range (empty string)");

		//-1 to account for null terminator.
		return (*this)[0];
	}

	/*	Returns the last character in this string (excluding null terminator).	
	*/
	const char& front() const
	{
		BS_ASSERT2(!empty(), "Index out of range (empty string)");

		//-1 to account for null terminator.
		return (*this)[0];
	}


	/*	Access to begin/end pointers, allowing this class to be used with STL algorithms
		and similar.
	*/
	char* begin()
	{
		return mData;
	}

	const char* cbegin() const
	{
		return mData;
	}

	char* end()
	{
		return mData + mStringLength;
	}

	const char* cend() const
	{
		return mData + mStringLength;
	}
	


	//Returns null terminated C string.
	const char* c_str() const
	{
		return mData;
	}
	

	/*	Returns true if this string contains no characters (excluding null terminator).
	*/
	bool empty() const
	{
		return mStringLength == 0;
	}

	/*	Length of string, not including null terminator.
	*/
	size_t length() const
	{
		return mStringLength;
	}

	/*	Length of string, not including null terminator.
	*/
	size_t size() const
	{
		return length();
	}

	/*	Sets this string to be empty.
	*/
	void clear()
	{
		setLength(0);
	}


	/*	Append a single character to the end of this string.
	*/
	void push_back(char c)
	{
		BS_ASSERT2(mStringLength + 2 <= Size, "Capacity exceeded");

		mData[mStringLength] = c;

		setLength(mStringLength + 1);
	}

	/*	Erase the last character in this string.
	*/
	void pop_back()
	{
		BS_ASSERT2(mStringLength > 0, "Popping an empty string");

		setLength(mStringLength - 1);
	}


	/*	Append a single character to the end of this string. This is the same as calling push_back.
	*/
	void append(char c)
	{
		push_back(c);
	}

	/*	Append null-terminated string.
	*/
	void append(const char* str)
	{
		const size_t paramLength = strlen(str);
		//Verify that we can fit own string, str, and null terminator.
		BS_ASSERT2(mStringLength + paramLength + 1 < Size, "Buffer overflow");

		//Copy str to end of current string.
		strcpy_s(mData + mStringLength, Size - mStringLength, str);

		//Update length by adding length of str.
		setLength(mStringLength + paramLength);
	}

	void append(const bsFixedSizeString& str)
	{
		const size_t paramLength = str.length();
		BS_ASSERT2(mStringLength + paramLength < Size, "Buffer overflow");

		//Copy str to end of current string.
		strcpy_s(mData + mStringLength, Size - mStringLength, str.mData);

		//Update length by adding length of str.
		setLength(mStringLength + paramLength);
	}

	/*	Replace content of this string with a printf-style string.
		Returns the number of characters written to this string.
		This overwrites the current content in this string.
	*/
	int printf(const char* format, ...)
	{
		va_list args;
		va_start(args, format);

		const int charsWritten = vsprintf_s(mData, format, args);

		va_end(args);

		setLength(charsWritten);

		return charsWritten;
	}

	/*	Append a printf-style string to the current content in this string.
		Returns the number of characters written to this string.
	*/
	int printfAppend(const char* format, ...)
	{
		va_list args;
		va_start(args, format);

		const int charsWritten = vsprintf_s(mData + length(), Size - mStringLength,
			format, args);

		va_end(args);

		setLength(length() + charsWritten);

		return charsWritten;
	}


	/*	Erase elements from and including index to but not including index+elementCount.
		Characters after the erased part of the string are moved to be next to the
		characters before the erased part.
		Erasing 'b' from "abc" results in "ac".
	*/
	void erase(size_t index, size_t elementCount = npos)
	{
		BS_ASSERT2(mStringLength > index, "Index out of range");

		if (index + elementCount > mStringLength || elementCount > mStringLength)
		{
			eraseAllFromIndex(index);
		}
		else
		{
			eraseFromIndex(index, elementCount);
		}
	}

	/*	Reverses this string.
	*/
	void reverse()
	{
		if (empty())
		{
			return;
		}

		char temp;
		for (size_t i = 0, j = mStringLength - 1; i < j; ++i, --j)
		{
			temp = (*this)[i];
			(*this)[i] = (*this)[j];
			(*this)[j] = temp;
		}
	}

	/*	Removes all occurences of the characters specified in the parameter from
		left end of this string.
	*/
	void trimLeft(const bsFixedSizeString& charsToTrim = " ")
	{
		//Find number of characters to remove.
		size_t numCharsToIgnore = 0;
		for (size_t i = 0; i < mStringLength; ++i)
		{
			if (charsToTrim.contains((*this)[i]))
			{
				++numCharsToIgnore;
			}
			else
			{
				//Found a char not in list of characters to trim, done searching.
				break;
			}
		}

		erase(0, numCharsToIgnore);
	}

	/*	Removes all occurences of the characters specified in the parameter from
		right end of this string.
	*/
	void trimRight(const bsFixedSizeString& charsToTrim = " ")
	{
		//Pop all characters that match input until a character not in the trim list is
		//found.
		while (!empty() && charsToTrim.contains(back()))
		{	
			pop_back();
		}
	}

	/*	Removes all occurences of the characters specified in the parameter from
		left and right ends of this string.
	*/
	void trim(const bsFixedSizeString& charsToTrim = " ")
	{
		trimLeft(charsToTrim);
		trimRight(charsToTrim);
	}


	/*	Returns true if this string contains at least one instance of charToFind.
	*/
	bool contains(char charToFind) const
	{
		return strchr(mData, charToFind) != nullptr;
	}
	
	/*	Returns true if this string contains at least one instance of the specified string.
	*/
	bool contains(const bsFixedSizeString& str) const
	{
		return contains(str.c_str());
	}

	/*	Returns true if this string contains at least one instance of the specified string.
	*/
	bool contains(const char* subStringToFind) const
	{
		return strstr(mData, subStringToFind) != nullptr;
	}

	/*	Returns true if this string contains any of the characters in the specified string.
	*/
	bool containsAnyOf(const char* chars) const
	{
		return strpbrk(mData, chars) != nullptr;
	}

	/*	Returns a substring of this string specified by the start index and length of
		substring.
	*/
	bsFixedSizeString subString(size_t startIndex, size_t stringLength) const
	{
		BS_ASSERT2(startIndex + stringLength < mStringLength, "Index out of range");

		return bsFixedSizeString(mData + startIndex, stringLength);
	}


	/*	Find first occurrence of stringToFind in this string.
		Returns the index of the first character if found, npos otherwise.
	*/
	size_t find(const bsFixedSizeString& stringToFind) const
	{
		const char* result = strstr(c_str(), stringToFind.c_str());
		if (result == nullptr)
		{
			return npos;
		}
		else
		{
			//Convert the result pointer to index.
			return result - mData;
		}
	}

	/*	Find first occurrence of char in this string.
		Returns the index of the character if found, npos otherwise.
	*/
	size_t find(char charToFind) const
	{
		const char* result = strchr(c_str(), charToFind);
		if (result == nullptr)
		{
			return npos;
		}
		else
		{
			//Convert the result pointer to index.
			return result - mData;
		}
	}


private:
	/*	Erase all elements from and including index.
	*/
	void eraseAllFromIndex(size_t index)
	{
		BS_ASSERT2(index < Size, "Index out of range");

		setLength(index - 1);
	}

	/*	Erase count elements from and including index.
	*/
	void eraseFromIndex(size_t index, size_t count)
	{
		BS_ASSERT2(index + count < mStringLength, "Index out of range");

		//Copy from after index+count to index.

		//Index of first element after the to-be-erased string.
		const size_t afterErasedIndex = index + count;
		//Length of string after the to-be-erased string.
		const size_t numElementsAfterErased = mStringLength - afterErasedIndex;
		
		memmove(mData + index, mData + afterErasedIndex, numElementsAfterErased);

		setLength(index + numElementsAfterErased);
	}

	/*	Sets the length of the string.
		Puts a null terminator at the end of the string and adjusts mStringLength.
	*/
	void setLength(size_t newLength)
	{
		BS_ASSERT2(newLength <= Size, "Length must be less than total size");

		mStringLength = newLength;
		//Set null terminator to one after end of string.
		mData[newLength] = '\0';

		BS_ASSERT2(*(mData + mStringLength) == '\0', "Null terminator missing (internal error)");
	}


	//Length of string (including null terminator). mData + mStringLength = null terminator.
	size_t	mStringLength;

	char	mData[Size];
};

//Typedefs for common sizes.
typedef bsFixedSizeString<16>	bsString16;
typedef bsFixedSizeString<32>	bsString32;
typedef bsFixedSizeString<64>	bsString64;
typedef bsFixedSizeString<128>	bsString128;
typedef bsFixedSizeString<256>	bsString256;
typedef bsFixedSizeString<512>	bsString512;
typedef bsFixedSizeString<1024>	bsString1024;




namespace std
{
	/*	Overloads for std::begin and std::end to allow bsFixedSizeString to be used with
		those functions.
	*/


	template <size_t Size>
	const char* begin(const bsFixedSizeString<Size>& str)
	{
		return str.cbegin();
	}

	template <size_t Size>
	char* begin(bsFixedSizeString<Size>& str)
	{
		return str.begin();
	}

	template <size_t Size>
	const char* end(const bsFixedSizeString<Size>& str)
	{
		return str.cend();
	}

	template <size_t Size>
	char* end(bsFixedSizeString<Size>& str)
	{
		return str.end();
	}



	/*	Overloads for std::hash to allow it to be used for unordered collection keys,
		ie unordered_map.
		Hashing algorithm taken from std::hash<std::string>.
	*/
	template <size_t Size>
	class hash<bsFixedSizeString<Size>>
		: public unary_function<bsFixedSizeString<Size>, size_t>
	{
	public:
		size_t operator()(const bsFixedSizeString<Size>& str) const
		{
			const size_t last = str.size();
			const size_t stride = 1 + last / 10;

			size_t val = 2166136261U;
			for(size_t first = 0; first < last; first += stride)
			{
				val = 16777619U * val ^ (size_t)str[first];
			}

			return val;
		}
	};
}
