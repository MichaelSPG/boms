#include "StdAfx.h"

#include "bsLog.h"

#ifndef BS_DISABLE_FEATURE_LOGGING

#include <pantheios/pantheios.hpp>
#include <pantheios/backends/bec.file.h>
#include <pantheios/frontends/fe.simple.h>


extern "C" const char PANTHEIOS_FE_PROCESS_IDENTITY[] = "TestApp";

PANTHEIOS_CALL(void) pantheios_be_file_getAppInit(int /* backEndId */, pan_be_file_init_t* init) /* throw() */
{
	init->flags |= PANTHEIOS_BE_INIT_F_NO_PROCESS_ID;
	init->flags |= PANTHEIOS_BE_INIT_F_NO_THREAD_ID;
	init->flags |= PANTHEIOS_BE_INIT_F_HIDE_DATE;
	init->flags |= PANTHEIOS_BE_INIT_F_HIGH_RESOLUTION;
	/*
	//All possible flags are listed here

	PANTHEIOS_BE_INIT_F_NO_PROCESS_ID
	PANTHEIOS_BE_INIT_F_NO_THREAD_ID
	PANTHEIOS_BE_INIT_F_NO_DATETIME
	PANTHEIOS_BE_INIT_F_NO_SEVERITY
	PANTHEIOS_BE_INIT_F_USE_SYSTEM_TIME
	PANTHEIOS_BE_INIT_F_DETAILS_AT_START
	PANTHEIOS_BE_INIT_F_USE_UNIX_FORMAT
	PANTHEIOS_BE_INIT_F_HIDE_DATE
	PANTHEIOS_BE_INIT_F_HIDE_TIME
	PANTHEIOS_BE_INIT_F_HIGH_RESOLUTION
	PANTHEIOS_BE_INIT_F_LOW_RESOLUTION
	*/
}

std::vector<std::function<void(const char*)>> bsLog::mCallbacks
	= std::vector<std::function<void(const char*)>>();

bool bsLog::init(bsLog::LogSeverity severityCeiling /*= pantheios::SEV_DEBUG*/)
{
	if (pantheios::init())
	{
		return false;
	}

	pantheios_be_file_setFilePath(PANTHEIOS_LITERAL_STRING("log.bsl"),
		PANTHEIOS_BE_FILE_F_TRUNCATE, PANTHEIOS_BE_FILE_F_TRUNCATE, PANTHEIOS_BEID_ALL);

	pantheios_fe_simple_setSeverityCeiling(severityCeiling);

	return true;
}

void bsLog::deinit()
{
	pantheios::pantheios_uninit();
}

void bsLog::logMessage(const char *message, bsLog::LogSeverity severity /*= SEV_INFORMATIONAL*/)
{
	pantheios::log(severity, message);

#ifndef BS_DISABLE_LOG_CALLBACKS
	//Execute all the callbacks with the message
	std::for_each(mCallbacks.cbegin(), mCallbacks.cend(),
		[&message](const std::function<void(const char*)>& func)
	{
		func(message);
	});
#endif
}

void bsLog::setLogLevel(LogSeverity severity)
{
	pantheios_fe_simple_setSeverityCeiling(severity);
}

int bsLog::getSeverityLevel()
{
	return pantheios_fe_simple_getSeverityCeiling();
}

#endif // BS_DISABLE_FEATURE_LOGGING
