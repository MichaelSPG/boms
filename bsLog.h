#pragma once


#include <vector>
#include <functional>


#ifndef BS_DISABLE_FEATURE_LOGGING

#ifdef _DEBUG
#pragma comment(lib, "pantheios.1.util.vc10.dll.debug.lib")
#pragma comment(lib, "pantheios.1.core.vc10.dll.debug.lib")
#pragma comment(lib, "pantheios.1.fe.simple.vc10.dll.debug.lib")
#pragma comment(lib, "pantheios.1.bec.file.WithCallback.vc10.dll.debug.lib")
#pragma comment(lib, "pantheios.1.be.file.vc10.dll.debug.lib")
#else
#pragma comment(lib, "pantheios.1.util.vc10.dll.lib")
#pragma comment(lib, "pantheios.1.core.vc10.dll.lib")
#pragma comment(lib, "pantheios.1.fe.simple.vc10.dll.lib")
#pragma comment(lib, "pantheios.1.bec.file.WithCallback.vc10.dll.lib")
#pragma comment(lib, "pantheios.1.be.file.vc10.dll.lib")
#endif


/*	Class for logging messages with severity levels to a file.
*/
class bsLog
{
public:
	//Severity levels for log messages.
	enum LogSeverity
	{
		//System is irrecoverable.
		SEV_EMERGENCY,

		//Attempting to recover from practically-irrecoverable condition (i.e. out of memory).
		SEV_ALERT,

		//Normal behavior of the program cannot be achieved, system likely to fail.
		SEV_CRITICAL,

		//Normal behavior of program cannot be achieved, system is likely to be able to
		//recover.
		SEV_ERROR,

		//Warning condition.
		SEV_WARNING,

		//Useful information for normal operation , e.g. "database connection achieved".
		SEV_NOTICE,

		//Non-essential information that may be useful when monitoring a system's health.
		SEV_INFORMATIONAL,

		//For debugging purposes only.
		SEV_DEBUG
	};


	/*	Initializes pantheios. Returns true on success.
		severityCeiling equals the lowest message severity which should be logged.
	*/
	static bool init(LogSeverity severityCeiling = SEV_DEBUG);

	static void deinit();

	/*	Logs a message to the log file.
		Also sends the message to all callback functions registered with the addCallback
		function.
	*/
	static void logMessage(const char* message, LogSeverity severity = SEV_INFORMATIONAL);

	/*	Sets the lowest level of severity to log.
		Messages with lower severity than the lowest severity will be thrown away.
	*/
	static void setLogLevel(LogSeverity severity);

	static int getSeverityLevel();

	/*	Adds a callback function which will receive every message logged.
	*/
	inline static void addCallback(const std::function<void(const char*)>& func)
	{
		mCallbacks.push_back(func);
	}

private:
	static std::vector<std::function<void(const char*)>>	mCallbacks;

	//Non-copyable
	bsLog(const bsLog&);
	void operator=(const bsLog&);
};

#else // BS_DISABLE_FEATURE_LOGGING

//All of this should get optimized away by the compiler.
class bsLog
{
public:
	enum LogSeverity
	{
		SEV_EMERGENCY,
		SEV_ALERT,
		SEV_CRITICAL,
		SEV_ERROR,
		SEV_WARNING,
		SEV_NOTICE,
		SEV_INFORMATIONAL,
		SEV_DEBUG
	};


	inline static bool init(LogSeverity) {return true;}

	inline static void deinit() {}

	inline static void logMessage(const char*, LogSeverity = SEV_INFORMATIONAL) {}

	inline static void setLogLevel(LogSeverity) {}

	inline static int getSeverityLevel() {return 0;}

	inline static void addCallback(const std::function<void(const char*)>&) {}

private:
	//Non-copyable
	bsLog(const bsLog&);
	void operator=(const bsLog&);
};
#endif // BS_DISABLE_LOGGING
