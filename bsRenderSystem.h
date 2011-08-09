#pragma once

#include "bsConfig.h"


/*	Base class for all render systems.
	
*/
class bsRenderSystem
{
public:
	virtual ~bsRenderSystem()
	{}

	virtual void renderOneFrame() = 0;
};
