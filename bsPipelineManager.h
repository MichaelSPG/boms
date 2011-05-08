#ifndef BS_PIPELINEMANAGER_H
#define BS_PIPELINEMANAGER_H

#include "bsConfig.h"

#include <vector>

#include <d3d11.h>

class bsRenderPass;


class bsPipelineManager
{
public:
	bsPipelineManager();
	~bsPipelineManager();

	

private:
	std::vector<bsRenderPass*>	mRenderPasses;
};

#endif // BS_PIPELINEMANAGER_H