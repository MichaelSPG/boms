#include "bsRenderStats.h"

#include <sstream>
#include <algorithm>


std::wstring bsRenderStats::getStatsString() const
{
	std::wstringstream statsString;
	statsString.setf(std::ios::fixed, std::ios::floatfield);
	statsString.precision(0);
	statsString << "FPS: " << mFps
		<< "\nAverage FPS: " << mAverageFps;
	statsString.precision(3);

	statsString << "\nFrame time: " << mFrameTimeMs << " ms"
		<< "\nAverage frame time: " << mAverageTimeMs << " ms"
		<< "\nMax frame time: " << mMaxFrameTimeMs << " ms"
		<< "\nMin frame time: " << mMinFrameTimeMs << " ms";
	statsString.precision(0);
	statsString << '\n' << mDurationToTrack * 0.001f;
	statsString.precision(3);
	statsString << " second min: " << mCurrentMin.first << " ms";
	statsString.precision(0);
	statsString << '\n' << mDurationToTrack * 0.001f;
	statsString.precision(3);
	statsString << " second max: " << mCurrentMax.first << " ms";

	return statsString.str();
}

void bsRenderStats::setFrameTime(const float timeMs)
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

	//Increase age
	for (unsigned int i = 0u; i < mTrackedTimes.size(); ++i)
	{
		mTrackedTimes[i].second += timeMs;
	}
	mCurrentMin.second += timeMs;
	mCurrentMax.second += timeMs;
	if (mCurrentMin.second > mDurationToTrack)
	{
		mCurrentMin.first = 1e5f;
	}
	if (mCurrentMax.second > mDurationToTrack)
	{
		mCurrentMax.first = 0.0f;
	}

	//Remove expired
	auto itr = std::remove_if(mTrackedTimes.begin(), mTrackedTimes.end(), 
		[&](const std::pair<float, float>& fp)
		{
			return fp.second > mDurationToTrack;
		});

	mTrackedTimes.erase(itr, mTrackedTimes.end());

	mTrackedTimes.push_back(std::pair<float, float>(timeMs, 0.0f));


	for (unsigned int i = 0u; i < mTrackedTimes.size(); ++i)
	{
		const auto& current = mTrackedTimes[i];

		if (current.first > mCurrentMax.first)
		{
			mCurrentMax = current;
		}
		else if (current.first < mCurrentMin.first)
		{
			mCurrentMin = current;
		}
	}
}
