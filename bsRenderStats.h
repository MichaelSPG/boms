#ifndef BSRENDERSTATS_H
#define BSRENDERSTATS_H

#include <string>


class bsRenderStats
{
public:

	bsRenderStats()
		: mFps(0.0f)
		, mAverageFps(1.0f)
		, mFrameTimeMs(0.0f)
		, mMaxFrameTimeMs(0.0f)
		, mMinFrameTimeMs(1e5f) //Should always be overwritten by the first call to setFrameTime()
		, mAverageWeight(1.0f)
	{}

	std::wstring getStatsString();

	inline void setFps(const float fps)
	{
		mFps = fps;

		mAverageWeight = 1.0f / mAverageFps;
		mAverageFps = mAverageFps * (1.0f - mAverageWeight) + fps * mAverageWeight;
	}

	inline void setFrameTime(const float timeMs)
	{
		mFrameTimeMs = timeMs;

		mAverageTimeMs = mAverageTimeMs * (1.0f - mAverageWeight) + timeMs * mAverageWeight;

		if (timeMs > mMaxFrameTimeMs)
		{
			mMaxFrameTimeMs = timeMs;
		}
		if (timeMs < mMinFrameTimeMs)
		{
			mMinFrameTimeMs = timeMs;
		}
	}

private:
	float	mFps;
	float	mAverageFps;
	float	mFrameTimeMs;
	float	mAverageTimeMs;

	float	mMaxFrameTimeMs;
	float	mMinFrameTimeMs;

	float	mAverageWeight;
};

#endif // BSRENDERSTATS_H