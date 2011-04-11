#ifndef BS_LOG_H
#define BS_LOG_H

#include "bsConfig.h"

#include <vector>
#include <functional>

#include <pantheios/pantheios.hpp>
#include <pantheios/backends/bec.file.h>
#include <pantheios/frontends/fe.simple.h>

PANTHEIOS_CALL(void) pantheios_be_file_getAppInit(int /* backEndId */, pan_be_file_init_t* init) /* throw() */;


#ifndef BS_DISABLE_LOGGING

class bsLog
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

#ifndef BS_DISABLE_LOG_CALLBACKS
	inline static void addCallback(const std::function<void(const char*)>& func)
	{
		mCallbacks.push_back(func);
	}

private:
	static std::vector<std::function<void(const char*)>>	mCallbacks;

#else
	inline static void addCallback(const std::function<void(const char*)>&) {}
#endif

	//Non-copyable
	bsLog(const bsLog&);
	void operator=(const bsLog&);
};
#else

//All of this should get optimized away by the compiler.
class bsLog
{
public:
	inline static bool init(pantheios::pan_severity_t) {return true;}

	inline static void deinit() {}

	inline static void logMessage(const char*,
		pantheios::pan_severity_t = pantheios::SEV_INFORMATIONAL) {}

	inline static void setLogLevel(pantheios::pan_severity_t) {}

	inline static int getSeverityLevel() {return 0;}

	inline static void addCallback(const std::function<void()>&) {}

private:
	//Non-copyable
	bsLog(const bsLog&);
	void operator=(const bsLog&);
};
#endif // BS_DISABLE_LOGGING

#endif // BS_LOG_H
