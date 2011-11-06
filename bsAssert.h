#pragma once

#include <sstream>

#include <Windows.h>

#include "bsLog.h"


#ifdef BS_ASSERT_NO_BREAK
#define BS_BREAKPOINT()
#else // BS_ASSERT_NO_BREAK
#define BS_BREAKPOINT() _asm { int 3 }
#endif // BS_ASSERT_NO_BREAK

#ifdef BS_DEBUG

/*	Log message with condition, custom text, file, line and function name, and then break.
*/
#define BS_ASSERT2(condition, text)														 \
do																						 \
{																						 \
	if (!(condition))																	 \
	{																					 \
		std::stringstream ss;															 \
		ss << "Assertion failed: " << #condition << '\n'								 \
		<< text << '\n'																	 \
		<< "File: " << __FILE__ << '(' << __LINE__ << ")\n"								 \
		<< "Function: " << __FUNCTION__;												 \
		bsLog::log(ss.str().c_str(), bsLog::SEV_ERROR);									 \
		ss << "\n";																		 \
		OutputDebugStringA(ss.str().c_str());											 \
																						 \
		ss << "\nDo you want to break? Cancel to exit.";								 \
		const int messageBoxRet = MessageBoxA(nullptr, ss.str().c_str(),				 \
			"Assertion failed", MB_ICONERROR | MB_YESNOCANCEL);							 \
		if (messageBoxRet == IDYES)														 \
		{																				 \
			BS_BREAKPOINT();															 \
		}																				 \
		else if (messageBoxRet == IDCANCEL)												 \
		{																				 \
			exit(1);																	 \
		}																				 \
	}																					 \
} while (false)

/*	Log message with condition, file, line and function name, and then break.
*/
#define BS_ASSERT(condition)															 \
do																						 \
{																						 \
	if (!(condition))																	 \
	{																					 \
		std::stringstream ss;															 \
		ss << "Assertion failed: " << #condition << '\n'								 \
		<< "File: " << __FILE__ << '(' << __LINE__ << ")\n"								 \
		<< "Function: " << __FUNCTION__;												 \
		bsLog::log(ss.str().c_str(), bsLog::SEV_ERROR);									 \
		ss << "\n";																		 \
		OutputDebugStringA(ss.str().c_str());											 \
																						 \
		ss << "\nDo you want to break? Cancel to exit.";								 \
		const int messageBoxRet = MessageBoxA(nullptr, ss.str().c_str(),				 \
		"Assertion failed", MB_ICONERROR | MB_YESNOCANCEL);								 \
		if (messageBoxRet == IDYES)														 \
		{																				 \
			BS_BREAKPOINT();															 \
		}																				 \
		else if (messageBoxRet == IDCANCEL)												 \
		{																				 \
			exit(1);																	 \
		}																				 \
	}																					 \
} while (false)

#else // BS_DEBUG
//Cast to void to avoid unused variable warning
#define BS_ASSERT2(condition, text) do { (void)(condition); (void)(text); } while (false)
#define BS_ASSERT(condition) do { (void)(condition); } while (false)
#endif // BS_DEBUG
