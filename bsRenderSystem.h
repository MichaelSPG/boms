#pragma once

struct bsFrameStatistics;


/*	Base class for all render systems.
	
*/
class bsRenderSystem
{
public:
	virtual ~bsRenderSystem()
	{}

	virtual void renderOneFrame(bsFrameStatistics& framStatistics) = 0;
};
