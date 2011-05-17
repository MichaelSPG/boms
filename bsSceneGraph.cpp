#include "bsSceneGraph.h"

#include <vector>

#include "bsSceneNode.h"
#include "bsTimer.h"
#include "bsResourceManager.h"
#include "bsCamera.h"
#include "bsLog.h"
#include "bsAssert.h"
#include "bsHavokManager.h"
#include "bsCoreCInfo.h"


bsSceneGraph::bsSceneGraph(bsDx11Renderer* renderer, bsResourceManager* resourceManager,
	bsHavokManager* havokManager, const bsCoreCInfo& cInfo)
	: mNumCreatedNodes(0)
	, mDx11Renderer(renderer)
	, mResourceManager(resourceManager)
	, mMaxTreeDepth(4)
	, mGraphicsWorld(nullptr)
{
	BS_ASSERT(renderer);
	BS_ASSERT(resourceManager);
	BS_ASSERT(havokManager);

	havokManager->createGraphicsWorld(true);
	mGraphicsWorld = havokManager->getGraphicsWorld();

	mCamera = new bsCamera(bsProjectionInfo(45.0f, 1000.0f, 0.1f,
		(float)cInfo.windowWidth / (float)cInfo.windowHeight), this, havokManager);

	bsLog::logMessage("Scene graph initialized successfully");
}

bsSceneGraph::~bsSceneGraph()
{
	for (unsigned int i = 0; i < mSceneNodes.size(); ++i)
	{
		delete mSceneNodes[i];
	}
	if (mCamera)
	{
		delete mCamera;
	}
}

bsSceneNode* bsSceneGraph::createSceneNode(const hkVector4& position
	/*= hkVector4(0.0f, 0.0f, 0.0f, 0.0f)*/)
{
	bsSceneNode* node = new bsSceneNode(position, getNewId(), this);

	mSceneNodes.push_back(node);

	return node;
}
