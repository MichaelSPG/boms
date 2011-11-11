#pragma once

#include <vector>
#include <functional>


#ifndef BS_DISABLE_FEATURE_LOGGING


/*	Class for logging messages with severity levels to a file.
	Multiple instances of this class is not supported.
*/
class bsLog
{
public:
	/*	Stack size used for logf formatting. Messages that require bigger buffer sizes
		than this will likely show up in the log destination as incomplete.
		If this is an issue, consider formatting the message manually and sending it to
		log instead of logf, as log does not have a problem with messages longer than this
		size.
	*/
	static const unsigned int kTempBufferSize = 5 * 1024;

	/*	The severity of a log message.
		When updating this, also update mSeverityStrings in the .cpp file.
	*/
	enum Severity
	{
		//For debugging purposes.
		SEV_DEBUG = 0,

		//Non-essential information which may be useful for monitoring a system's health.
		SEV_INFO = 1,

		//Warning condition.
		SEV_WARNING = 2,

		//Normal behavior of system cannot be achieved, but system is likely to be able
		//to recover.
		SEV_ERROR = 3,

		//System is likely to fail.
		SEV_CRICICAL = 4,

		//Don't use. Used for determining array size of severity level to string mapping.
		SEVERITY_COUNT = 5,
	};

	/*	Flags for configuring string formatting.
		Any flags that are not specified will default to disabled.
		These can be combined used bit operators.
	*/
	enum Flags
	{
		//YYYY/MM/DD
		DATE_ENABLED = 1 << 0,

		//HH:MM:SS
		TIMESTAMP_SECS = 1 << 1,
		//HH:MM:SS:MS0 where MS0 is milliseconds with 3 digits.
		TIMESTAMP_MILLISECS = (1 << 2) | TIMESTAMP_SECS,

		//Outputs severity level as text.
		SEVERITY_AS_TEXT = 1 << 3,
		//Outputs severity level as an int.
		SEVERITY_AS_INT = 1 << 4,

		//Output thread ID, format: 0x1234.
		THREAD_ID_ENABLED = 1 << 5,

		//Does not close the file provided in the constructor upon destruction of this object.
		DONT_CLOSE_FILE = 1 << 6,
	};


	/*	Call this function before logging any messages.
		file must point to a valid FILE struct.
		minSeverity represents the minimum severity of messages to log to the file.
		flags represents options used for formatting timestamps and similar.

		This function is NOT thread safe.
	*/
	static void init(FILE* file, Severity minSeverity = SEV_DEBUG, unsigned char flags = 0);

	/*	Call this function when done logging messages. It will close the file (unless 
		the DONT_CLOSE_FILE flag has been specified).
	*/
	static void deinit();


	/*	Logs a message to file with the specified severity level.
		
		If the specified severity level is below the current minimum severity level, the
		message is ignored.

		Also sends the message to all callback functions registered with the addCallback
		function.

		This function is thread safe.
	*/
	static void log(const char* message, Severity severity = SEV_INFO);

	/*	Logs a formatted message to file with the specified severity level.
		This function supports printf-style logging, using the exact same syntax as printf.
		Keep in mind that a buff is stack allocated whenever this function is called, with
		size equal kTempBufferSize, so very long messages may need to be formatted externally.

		If the specified severity level is below the current minimum severity level, the
		message is ignored.

		Also sends the message to all callback functions registered with the addCallback
		function.

		This function is thread safe.
	*/
	static void logf(Severity severity, const char* format, ...);



	/*	Get currently active flags (from Flags enum).
	*/
	static inline unsigned short getFlags()
	{
		return mMinSevAndFlags & 0xFFF;
	}

	/*	Set flags for formatting messages.

		This function is NOT thread safe.
	*/
	static inline void setFlags(unsigned short flags)
	{
		//Set parameter to lowest 12 bits, keep highest 4 (severity).
		mMinSevAndFlags = (mMinSevAndFlags & 0xF000) | flags;
	}

	/*	Get current minimum severity level.
		Messages whose severity level is below this are ignored.
	*/
	static inline Severity getMinSeverity()
	{
		return (Severity)((mMinSevAndFlags & 0xF000) >> 12);
	}

	/*	Set minimum severity level required for messages to be logged.

		This function is NOT thread safe.
	*/
	static inline void setMinSeverity(Severity severity)
	{
		//Set parameter as highest 4 bytes, leave rest (flags) unchanged.
		mMinSevAndFlags = ((mMinSevAndFlags & 0xFFF) | (unsigned short)(severity << 12));
	}


	/*	Adds a callback function which will receive every message logged.
	*/
	static inline void addCallback(const std::function<void(const char*)>& func)
	{
		mCallbacks.push_back(func);
	}

	static inline void clearAllCallbacks()
	{
		mCallbacks.clear();
	}

private:
	//Non-copyable.
	bsLog(const bsLog&);
	void operator=(const bsLog&);


	//Highest 4 bits = severity level, lowest 12 bits = flags.
	static unsigned short	mMinSevAndFlags;

	//The file messages are being logged to.
	static FILE* mFile;

	//Text versions of Severity enum.
	static const char* mSeverityStrings[SEVERITY_COUNT];

	static std::vector<std::function<void(const char*)>>	mCallbacks;
};

#else // BS_DISABLE_FEATURE_LOGGING

//All of this should get optimized away by the compiler.
class bsLog
{
public:
	enum Severity
	{
		SEV_DEBUG = 0,
		SEV_INFO = 1,
		SEV_WARNING = 2,
		SEV_ERROR = 3,
		SEV_CRICICAL = 4,
		SEVERITY_COUNT = 5,
	};

	enum Flags
	{
		DATE_ENABLED = 1 << 0,
		TIMESTAMP_SECS = 1 << 1,
		TIMESTAMP_MILLISECS = (1 << 2) | TIMESTAMP_SECS,
		SEVERITY_AS_TEXT = 1 << 3,
		SEVERITY_AS_INT = 1 << 4,
		THREAD_ID_ENABLED = 1 << 5,
		DONT_CLOSE_FILE = 1 << 6,
	};


	static void init(FILE*, Severity = SEV_DEBUG, unsigned char = 0)
	{}

	static void deinit()
	{}

	static void logMessage(const char*, Severity = SEV_INFO)
	{}

	static void logf(Severity, const char*, ...)
	{}

	static inline unsigned short getFlags()
	{}

	static inline void setFlags(unsigned short)
	{}

	static inline Severity getMinSeverity()
	{}

	static inline void setMinSeverity(Severity)
	{}

	static inline void addCallback(const std::function<void(const char*)>&)
	{}

private:
	//Non-copyable
	bsLog(const bsLog&);
	void operator=(const bsLog&);
};
#endif // BS_DISABLE_LOGGING
