#include "Log.h"

Severity Log::logLevel = LOG_SEV_DEBUG;

void Log::init(Severity logLvl /*= LOG_SEV_DEBUG*/)
{
	if (pantheios::init())
		throw (std::exception("Failed to initialize Pantheios."));

	

	pantheios_be_file_setFilePath(PANTHEIOS_LITERAL_STRING("log.log"), PANTHEIOS_BE_FILE_F_TRUNCATE,
		PANTHEIOS_BE_FILE_F_TRUNCATE, PANTHEIOS_BEID_ALL);

	logLevel = logLvl;

	pantheios_fe_simple_setSeverityCeiling(logLevel);
}

void Log::log(const char *message, Severity severity /*= LOG_SEV_INFORMATIONAL*/)
{
	if (severity > logLevel)
		return;

	switch (severity)
	{
	case LOG_SEV_EMERGENCY:
		pantheios::log(pantheios::emergency, message);
		break;

	case LOG_SEV_ALERT:
		pantheios::log(pantheios::alert, message);
		break;

	case LOG_SEV_CRITICAL:
		pantheios::log(pantheios::critical, message);
		break;

	case LOG_SEV_ERROR:
		pantheios::log(pantheios::error, message);
		break;

	case LOG_SEV_WARNING:
		pantheios::log(pantheios::warning, message);
		break;

	case LOG_SEV_NOTICE:
		pantheios::log(pantheios::notice, message);
		break;

	case LOG_SEV_INFORMATIONAL:
		pantheios::log(pantheios::informational, message);
		break;

	case LOG_SEV_DEBUG:
		pantheios::log(pantheios::debug, message);
		break;
	}
}
