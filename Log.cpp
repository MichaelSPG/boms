#include "Log.h"

extern "C" const char PANTHEIOS_FE_PROCESS_IDENTITY[] = "TestApp";

PANTHEIOS_CALL(void) pantheios_be_file_getAppInit(int /* backEndId */, pan_be_file_init_t* init) /* throw() */
{
	init->flags |= PANTHEIOS_BE_INIT_F_NO_PROCESS_ID;
	init->flags |= PANTHEIOS_BE_INIT_F_NO_THREAD_ID;
	init->flags |= PANTHEIOS_BE_INIT_F_HIDE_DATE;
	init->flags |= PANTHEIOS_BE_INIT_F_HIGH_RESOLUTION;
	/*
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

#ifndef BS_DISABLE_LOGGING

bool Log::init(pantheios::pan_severity_t severity /*= pantheios::SEV_DEBUG*/)
{
	if (pantheios::init())
	{
		return false;
	}

	pantheios_be_file_setFilePath(PANTHEIOS_LITERAL_STRING("log.bsl"),
		PANTHEIOS_BE_FILE_F_TRUNCATE, PANTHEIOS_BE_FILE_F_TRUNCATE, PANTHEIOS_BEID_ALL);

	pantheios_fe_simple_setSeverityCeiling(severity);

	return true;
}

void Log::deinit()
{
	pantheios::pantheios_uninit();
}

void Log::logMessage(const char *message,
	pantheios::pan_severity_t severity /*= pantheios::SEV_INFORMATIONAL*/)
{
	pantheios::log(severity, message);
}

#endif // BS_DISABLE_LOGGING