#pragma once


/*	Base class for all render systems.
	
*/
class bsRenderSystem
{
public:
	virtual ~bsRenderSystem()
	{}

	virtual void renderOneFrame() = 0;
};
