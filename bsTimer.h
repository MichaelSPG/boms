#ifndef BS_TIMER_H
#define BS_TIMER_H

#include <windows.h>

/**	Simple timer.
	Use bsTimer::getTimeMilliSeconds to get millisecond intervals.
	Uses no CPU cycles when idle.
	Will likely be inaccurate if power saving or similar is active.
*/
class bsTimer
{
public:
	bsTimer()
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		mOneOverFrequency = (float)(1.0 / (double)frequency.QuadPart);

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

#endif // BS_TIMER_H