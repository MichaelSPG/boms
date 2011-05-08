#ifndef BS_RENDERER_H
#define BS_RENDERER_H

#include "bsConfig.h"


class bsRenderSystem
{
public:
	virtual ~bsRenderSystem()
	{}

	virtual void renderOneFrame() = 0;
};

#endif // BS_RENDERER_H