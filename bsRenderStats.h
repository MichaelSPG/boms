#ifndef BS_RENDERSTATS_H
#define BS_RENDERSTATS_H

#include <string>
#include <map>


class bsRenderStats
{
public:

	bsRenderStats()
		: mFps(0.0f)
		, mAverageFps(1.0f)
		, mFrameTimeMs(0.0f)
		, mAverageWeight(1.0f)
		, mHistoryDuration(10000.0f)

		//These 2 will be updated by the setFrameTime functions so we can just set them to
		//silly values here to force the update to happen during the first function call
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
	inline void setHistoryLength(const float durationMs)
	{
		mHistoryDuration = durationMs;
	}

private:
	float	mFps;
	float	mAverageFps;
	float	mFrameTimeMs;
	float	mAverageTimeMs;

	float	mAverageWeight;
	float	mHistoryDuration;

	//pair<frame time, age in ms>
	std::pair<float, float>	mCurrentMin;
	std::pair<float, float>	mCurrentMax;
	std::map<float, float>	mTrackedTimes;
};

#endif // BS_RENDERSTATS_H
