#ifndef BS_RENDERER_H
#define BS_RENDERER_H

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

#endif // BS_RENDERER_H