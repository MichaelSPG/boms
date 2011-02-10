#ifndef LOG_H_____
#define LOG_H_____

#include <pantheios/pantheios.hpp>
#include <pantheios/backends/bec.file.h>
#include <pantheios/frontends/fe.simple.h>

#include "ToString.h"


//http://blog.pantheios.org/2010/10/choosing-severity-levels.html
enum Severity
{
	LOG_SEV_EMERGENCY,
	LOG_SEV_ALERT,
	LOG_SEV_CRITICAL,
	LOG_SEV_ERROR,
	LOG_SEV_WARNING,
	LOG_SEV_NOTICE,
	LOG_SEV_INFORMATIONAL,
	LOG_SEV_DEBUG
};


class Log
{
public:
	/**	
		@param logLvl The lowest severity of messages to log.
	*/
	static void init(Severity logLvl = LOG_SEV_DEBUG);


	/**	Logs a message to the log file.
		If severity is less than current logLevel, the message will be ignored.
	*/
	static void log(const char *message, Severity severity = LOG_SEV_INFORMATIONAL);

	inline static void setLogLevel(Severity severity)
	{
		logLevel = severity;
	}

	inline static Severity getLogLevel()
	{
		return logLevel;
	}

private:
	static Severity logLevel;
};

#endif