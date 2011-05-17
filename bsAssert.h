#ifndef BS_ASSERT_H
#define BS_ASSERT_H

#include "bsConfig.h"

#include <sstream>

#include "bsLog.h"


#ifdef BS_ASSERT_NO_BREAK
#define BS_BREAKPOINT()
#else // BS_ASSERT_NO_BREAK
#define BS_BREAKPOINT() _asm { int 3 }
#endif // BS_ASSERT_NO_BREAK

#if BS_DEBUG_LEVEL > 0
/*	Log message with condition, custom text, file, line and function name, and then break.
*/
#define BS_ASSERT2(condition, text)											\
do																			\
{																			\
	if (!(condition))														\
	{																		\
		std::stringstream ss;												\
		ss << "Assertion failed: " << #condition << '\n'					\
		<< text << '\n'														\
		<< "File: " << __FILE__ << '(' << __LINE__ << ")\n"					\
		<< "Function: " << __FUNCTION__;									\
		bsLog::logMessage(ss.str().c_str(), pantheios::SEV_ERROR);			\
		BS_BREAKPOINT();													\
	}																		\
} while (false)

/*	Log message with condition, file, line and function name, and then break.
*/
#define BS_ASSERT(condition)												\
do																			\
{																			\
	if (!(condition))														\
	{																		\
		std::stringstream ss;												\
		ss << "Assertion failed: " << #condition << '\n'					\
		<< "File: " << __FILE__ << '(' << __LINE__ << ")\n"					\
		<< "Function: " << __FUNCTION__;									\
		bsLog::logMessage(ss.str().c_str(), pantheios::SEV_ERROR);			\
		BS_BREAKPOINT();													\
	}																		\
} while (false)

#else // BS_DEBUG_LEVEL > 0
#define BS_ASSERT2(condition, text) do { (void)(condition); (void)(text); } while (false)
#define BS_ASSERT(condition) do { (void)(condition); } while (false)
#endif // BS_DEBUG_LEVEL > 0

#endif // BS_ASSERT_H
