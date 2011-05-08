#ifndef BS_RENDERPASS_H
#define BS_RENDERPASS_H

#include "bsConfig.h"

class bsRenderTarget;


struct bsRenderPass
{
	bsRenderPass()
	{
		memset(this, 0, sizeof(*this));
	}

	bsRenderTarget*	mRenderTarget;

	float clearColor[];

};

#endif // BS_RENDERPASS_H