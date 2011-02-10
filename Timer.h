#ifndef TIMER_H
#define TIMER_H

#include <windows.h>

/**	Simple timer.
	Call Timer::start(), then use Timer::getTimeMilliSeconds to get milliseconds since
	Timer::start() was called.
	Uses no CPU cycles when idle.
*/
class Timer
{
public:
	void start()
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		//
		mOneOverFrequency = 1.0f / (float)frequency.QuadPart;

		QueryPerformanceCounter(&mStart);
	}

	void reset()
	{
		QueryPerformanceCounter(&mStart);
	}

	float getTimeMilliSeconds()
	{
		QueryPerformanceCounter(&mEnd);
		return (1e3f * (mEnd.QuadPart - mStart.QuadPart)) * mOneOverFrequency;
	}

private:
	LARGE_INTEGER mStart;
	LARGE_INTEGER mEnd;
	float mOneOverFrequency;
};

#endif



/*
class Timer2
{
public:
	Timer2() {}
	~Timer2() {}

	inline void start()
	{
		mStartTime = GetTickCount();
	}

	inline DWORD getTime()
	{
		return GetTickCount() - mStartTime;
	}
	
private:
	DWORD mStartTime;
};
*/