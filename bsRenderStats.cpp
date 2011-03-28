#include "bsRenderStats.h"

#include <sstream>


std::wstring bsRenderStats::getStatsString()
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

	return statsString.str();
}
