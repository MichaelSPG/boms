#ifndef BS_CONFIG_H
#define BS_CONFIG_H

#include <exception>

/*	5 for maximum debug level, 0 for minimum.
*/
#ifndef BS_DEBUG_LEVEL
#ifdef _DEBUG
#define BS_DEBUG_LEVEL 5
#else
#define BS_DEBUG_LEVEL 0
#endif // _DEBUG
#endif // BS_DEBUG_LEVEL

#if BS_DEBUG_LEVEL > 5 || BS_DEBUG_LEVEL < 0
#error BS_DEBUG_LEVEL must be between 0 and 5 (inclusive)
#endif // BS_DEBUG_LEVEL > 5 || BS_DEBUG_LEVEL < 0




//Disables all logging to log file.
//#define BS_DISABLE_LOGGING


#endif // BS_CONFIG_H