#ifndef BSRENDERSTATS_H
#define BSRENDERSTATS_H

#include <string>
#include <vector>


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
		, mDurationToTrack(10000.0f)
		, mCurrentMin(1e5f, 0.0f)
		, mCurrentMax(0.0f, 0.0f)
	{}

	std::wstring getStatsString() const;

	inline void setFps(const float fps)
	{
		mFps = fps;

		mAverageWeight = 1.0f / mAverageFps;
		mAverageFps = mAverageFps * (1.0f - mAverageWeight) + fps * mAverageWeight;
	}

	void setFrameTime(const float timeMs);

	//Default: 10000.0f (10 seconds)
	inline void setDurationToTrack(const float durationMs)
	{
		mDurationToTrack = durationMs;
	}

private:
	float	mFps;
	float	mAverageFps;
	float	mFrameTimeMs;
	float	mAverageTimeMs;

	float	mMaxFrameTimeMs;
	float	mMinFrameTimeMs;

	float	mAverageWeight;

	float	mDurationToTrack;
	//pair<value, age in ms>
	std::pair<float, float>	mCurrentMin;
	std::pair<float, float>	mCurrentMax;

	std::vector<std::pair<float, float>>	mTrackedTimes;
};

#endif // BSRENDERSTATS_H