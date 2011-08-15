#pragma once

#include <string>

#include <Windows.h>


/*	Contains utility functions for dealing with Windows API.
*/

namespace bs
{

/*	Converts an error code to a readable string.
	The error code this function takes is usually gotten from GetLastError().
*/
std::string winApiErrorCodeToString(DWORD errorCode)
{
	char* buffer = nullptr;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&buffer, 0, nullptr);

	const std::string errorMessage(buffer, buffer + strlen(buffer));

	LocalFree(buffer);

	return errorMessage;
}

}