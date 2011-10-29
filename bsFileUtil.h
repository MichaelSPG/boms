#pragma once

#include <Windows.h>


namespace bsFileUtil
{
	/*	Checks if the specified directory name is actually a directory.
		Returns true if it exists and is a directory, false otherwise.
	*/
	bool directoryExists(const char* directoryName)
	{
		const DWORD attributes = GetFileAttributes(directoryName);

		return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	bool fileExists(const char* fileName)
	{
		const DWORD attributes = GetFileAttributes(fileName);

		return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);

	}

	time_t filetimeToTime_t(const FILETIME& ft)
	{
		//A FILETIME is the number of 100-nanosecond intervals since January 1, 1601.
		//A time_t is the number of 1-second intervals since January 1, 1970.
		ULARGE_INTEGER ull;
		ull.LowPart = ft.dwLowDateTime;
		ull.HighPart = ft.dwHighDateTime;
		return ull.QuadPart / 10000000ULL - 11644473600ULL;
	}

	/*	Returns a time_t describing when a file was last modified.
	*/
	time_t lastModifiedTime(const char* fileName)
	{
		WIN32_FILE_ATTRIBUTE_DATA fileAttributes;
		GetFileAttributesEx(fileName, GetFileExInfoStandard, &fileAttributes);

		return filetimeToTime_t(fileAttributes.ftLastWriteTime);
	}
}
