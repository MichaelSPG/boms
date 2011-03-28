#ifndef BS_LOG_H
#define BS_LOG_H

#include "bsConfig.h"



#include <pantheios/pantheios.hpp>
#include <pantheios/backends/bec.file.h>
#include <pantheios/frontends/fe.simple.h>

PANTHEIOS_CALL(void) pantheios_be_file_getAppInit(int /* backEndId */, pan_be_file_init_t* init) /* throw() */;

#ifndef BS_DISABLE_LOGGING

class Log
{
public:
	/**	Returns true on success.
		The lowest message severity to log.
	*/
	static bool init(pantheios::pan_severity_t severity = pantheios::SEV_DEBUG);

	static void deinit();

	/**	Logs a message to the log file.
	*/
	static void logMessage(const char* message,
		pantheios::pan_severity_t severity = pantheios::SEV_INFORMATIONAL);

	inline static void setLogLevel(pantheios::pan_severity_t severity)
	{
		pantheios_fe_simple_setSeverityCeiling(severity);
	}

	inline static int getSeverityLevel()
	{
		return pantheios_fe_simple_getSeverityCeiling();
	}
};
#else

//All of this should get optimized away by the compiler.
class Log
{
public:
	inline static bool init(pantheios::pan_severity_t) {return true;}

	inline static void deinit() {}

	inline static void logMessage(const char*,
		pantheios::pan_severity_t = pantheios::SEV_INFORMATIONAL) {}

	inline static void setLogLevel(pantheios::pan_severity_t) {}

	inline static int getSeverityLevel() {return 0;}
};
#endif // BS_DISABLE_LOGGING

#endif // BS_LOG_H