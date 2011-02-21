#ifndef LOG_H_____
#define LOG_H_____

#include <pantheios/pantheios.hpp>
#include <pantheios/backends/bec.file.h>
#include <pantheios/frontends/fe.simple.h>

PANTHEIOS_CALL(void) pantheios_be_file_getAppInit(int /* backEndId */, pan_be_file_init_t* init) /* throw() */;


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

#endif