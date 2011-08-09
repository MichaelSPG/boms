#pragma once

#include <windows.h>

/*	Simple timer.
	Use bsTimer::getTimeMilliSeconds to get millisecond intervals.
	Uses no CPU cycles when idle.
	Will likely be inaccurate if power saving or similar is active.

	Usage looks something like this:
	bsTimer timer;
	float start = timer.getTimeMilliSeconds();
	doSomeWork();
	float duration = timer.getTimeMilliSeconds() - start;
	//duration now contains time doSomeWork() took in milliseconds.
*/
class bsTimer
{
public:
	inline bsTimer()
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		mOneOverFrequency = (float)(1.0 / (double)frequency.QuadPart);

		QueryPerformanceCounter(&mStart);
	}

	inline void reset()
	{
		QueryPerformanceCounter(&mStart);
	}

	inline float getTimeMilliSeconds() const
	{
		QueryPerformanceCounter(&mEnd);
		return (1e3f * (mEnd.QuadPart - mStart.QuadPart)) * mOneOverFrequency;
	}

private:
	LARGE_INTEGER mStart;
	mutable LARGE_INTEGER mEnd;
	float mOneOverFrequency;
};
