#include "bsRenderStats.h"

#include <sstream>
#include <algorithm>

#include "bsTemplates.h"


std::wstring bsRenderStats::getStatsString() const
{
	std::wstringstream statsString;
	statsString.setf(std::ios::fixed, std::ios::floatfield);
	statsString.precision(0);
	statsString << "FPS: " << mFps
		<< "\nAverage FPS: " << mAverageFps;
	statsString.precision(3);

	statsString << "\nFrame time: " << mFrameTimeMs << " ms"
		<< "\nAverage frame time: " << mAverageTimeMs << " ms";
	statsString.precision(0);
	statsString << '\n' << mHistoryDuration * 0.001f;
	statsString.precision(3);
	statsString << " second min: " << mCurrentMin.first << " ms";
	statsString.precision(0);
	statsString << '\n' << mHistoryDuration * 0.001f;
	statsString.precision(3);
	statsString << " second max: " << mCurrentMax.first << " ms";

	return statsString.str();
}

void bsRenderStats::setFrameTime(const float timeMs)
{
	mFrameTimeMs = timeMs;
	mAverageTimeMs = mAverageTimeMs * (1.0f - mAverageWeight) + timeMs * mAverageWeight;

	mTrackedTimes.insert(std::make_pair<float, float>(timeMs, 0.0f));

	//Increase age
	for (auto itr = mTrackedTimes.begin(), end = mTrackedTimes.end(); itr != end; ++itr)
	{
		itr->second += timeMs;
	}

	mCurrentMin.second += timeMs;
	mCurrentMax.second += timeMs;

	//Update current if expired
	if (mCurrentMin.second > mHistoryDuration)
	{
		mCurrentMin = *mTrackedTimes.begin();
	}
	if (mCurrentMax.second > mHistoryDuration)
	{
		mCurrentMax = *mTrackedTimes.rbegin();
	}

	//Remove expired
	bsT::map_remove_if(mTrackedTimes,
		[&](const std::pair<float, float>& time)
	{
		return time.second > mHistoryDuration;
	});

	//Check if current is actually the value it should be, and update it if not
	if (!mTrackedTimes.empty())
	{
		if (mCurrentMax.first < mTrackedTimes.rbegin()->first)
		{
			mCurrentMax = *mTrackedTimes.rbegin();
		}
		if (mCurrentMin.first > mTrackedTimes.begin()->first)
		{
			mCurrentMin = *mTrackedTimes.begin();
		}
	}
}
