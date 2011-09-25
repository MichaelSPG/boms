#pragma once

#include <string>

#include <Windows.h>


/*	Contains utility functions for dealing with Windows API.
*/

namespace bsWindowsUtils
{

/*	Converts an error code to a readable string.
	The error code this function takes is usually gotten from GetLastError().
*/
inline std::string winApiErrorCodeToString(DWORD errorCode)
{
	char* buffer = nullptr;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&buffer, 0, nullptr);

	const std::string errorMessage(buffer, buffer + strlen(buffer));

	LocalFree(buffer);

	return errorMessage;
}


const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push, 8)
struct ThreadNameInfo
{
	DWORD dwType; // Must be 0x1000.
	const char* name; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
};
#pragma pack(pop)

/*	Sets the name of the thread with the specified ID.
	The thread ID can be obtained by calling GetThreadId() with a thread handle, or it can
	be set to -1 to name the calling thread.

	The thread name is probably useless for anything but inspecting with a debugger.
	
	The name is limited to 31 characters + null terminator.
*/
inline void setThreadName(DWORD threadId, const char* threadName)
{
	if (IsDebuggerPresent())
	{
		ThreadNameInfo threadNameInfo;
		threadNameInfo.dwType = 0x1000;
		threadNameInfo.name = threadName;
		threadNameInfo.dwThreadID = threadId;
		threadNameInfo.dwFlags = 0;

		RaiseException(MS_VC_EXCEPTION, 0, sizeof(ThreadNameInfo) / sizeof(ULONG_PTR),
			(ULONG_PTR*)&threadNameInfo);
	}
}

}