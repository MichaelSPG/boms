#include "StdAfx.h"

#include "bsLog.h"
#include "bsAssert.h"


#ifndef BS_DISABLE_FEATURE_LOGGING

unsigned short bsLog::mMinSevAndFlags = 0;

FILE* bsLog::mFile = nullptr;

const char* bsLog::mSeverityStrings[bsLog::SEVERITY_COUNT] =
{
	"Debug   ",
	"Info    ",
	"Warning ",
	"Error   ",
	"Critical",
};

std::vector<std::function<void(const char*)>> bsLog::mCallbacks
	= std::vector<std::function<void(const char*)>>();


void bsLog::init(FILE* file, Severity minSeverity /*= INFO*/, unsigned char flags /*= 0*/)
{
	BS_ASSERT2(file, "Invalid file");

	mFile = file;
	mMinSevAndFlags = (unsigned short)((minSeverity << 12) | flags);
}

void bsLog::deinit()
{
	if (!(getFlags() & DONT_CLOSE_FILE))
	{
		fclose(mFile);
	}
}

void bsLog::log(const char* message, Severity severity /*= INFO*/)
{
	//Only log the message if severity is over current severity level.
	if (severity < getMinSeverity())
	{
		return;
	}

	/*	Need exactly 47 characters when everything is enabled, unless thread ID requires
		more than 4 digits.
		Allocate some extra bytes to make sure everything goes smoothly in the event of
		something unpredicted happens.
	*/
	const size_t bufferSize = 64;
	char formatBuffer[bufferSize] = { 0 };

	//Currently active flags.
	const unsigned short flags = getFlags();

	//True if any extra formatting is enabled. Will need [] and something between the two.
	const bool startEndBrace = (flags & (TIMESTAMP_SECS | TIMESTAMP_MILLISECS |
		DATE_ENABLED | SEVERITY_AS_TEXT | SEVERITY_AS_INT | THREAD_ID_ENABLED)) != 0;

	if (startEndBrace)
	{
		//Characters written to formatBuffer.
		size_t written = 0;

		formatBuffer[0] = '[';
		++written;

		//Get system time to we can provide a timestamp.
		SYSTEMTIME localTime;
		GetLocalTime(&localTime);

		//Date.
		if (flags & DATE_ENABLED)
		{
			written += sprintf_s(formatBuffer + written, bufferSize - written,
				"%04u/%02u/%02u", localTime.wYear, localTime.wMonth, localTime.wDay);
		}

		//Time (HH:MM:SS).
		if (flags & (TIMESTAMP_SECS | TIMESTAMP_MILLISECS))
		{
			if (written != 1)
			{
				//Date has been written, need a comma.
				formatBuffer[written] = ',';
				formatBuffer[written + 1] = ' ';
				written += 2;
			}

			written += sprintf_s(formatBuffer + written, bufferSize - written,
				"%02u:%02u:%02u", localTime.wHour, localTime.wMinute, localTime.wSecond);
		}
		//Milliseconds.
		if (flags & TIMESTAMP_MILLISECS)
		{
			written += sprintf_s(formatBuffer + written, bufferSize - written,
				":%03u", localTime.wMilliseconds);
		}

		//Severity level.
		if (flags & SEVERITY_AS_TEXT)
		{
			if (written != 1)
			{
				//Date or time has been written, need a comma.
				formatBuffer[written] = ',';
				formatBuffer[written + 1] = ' ';
				written += 2;
			}
			written += sprintf_s(formatBuffer + written, bufferSize - written,
				mSeverityStrings[severity]);
		}
		else if (flags & SEVERITY_AS_INT)
		{
			if (written != 1)
			{
				//Date or time has been written, need a comma.
				formatBuffer[written] = ',';
				formatBuffer[written + 1] = ' ';
				written += 2;
			}
			written += sprintf_s(formatBuffer + written, bufferSize - written,
				"%u", severity);
		}

		//Thread ID.
		if (flags & THREAD_ID_ENABLED)
		{
			if (written != 1)
			{
				//Something has been written, need a comma.
				formatBuffer[written] = ',';
				formatBuffer[written + 1] = ' ';
				written += 2;
			}

			const unsigned long threadId = GetThreadId(GetCurrentThread());

			written += sprintf_s(formatBuffer + written, bufferSize - written,
				"0x%04X", threadId);
		}

		formatBuffer[written] = ']';
		formatBuffer[written + 1] = ' ';
		formatBuffer[written + 2] = '\0';
	}

	//Print the actual message.
	fprintf(mFile, "%s%s\n", formatBuffer, message);

	//Flush immediately, don't want to lose unflushed data if the program crashes.
	fflush(mFile);


	//Call registered callback functions.
	for (unsigned int i = 0; i < mCallbacks.size(); ++i)
	{
		mCallbacks[i](message);
	}
}

void bsLog::logf(Severity severity, const char* format, ...)
{
	//Only log the message if severity is over current severity level.
	if (severity < getMinSeverity())
	{
		return;
	}

	//Get ... args.
	va_list args;
	va_start(args, format);

	char buffer[kTempBufferSize];

	//Write args into buffer.
	vsprintf_s(buffer, format, args);

	va_end(args);
	
	//Log the formatted message through the normal log function.
	log(buffer, severity);
}


#endif // BS_DISABLE_FEATURE_LOGGING
