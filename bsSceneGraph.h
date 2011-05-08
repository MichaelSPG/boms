#ifndef BS_SCENE_GRAPH_H
#define BS_SCENE_GRAPH_H

#include <vector>
#include <memory>

#include "bsMath.h"

class bsResourceManager;
class bsSceneNode;
class bsCamera;
class bsDx11Renderer;
class bsRenderable;
class bsHavokManager;
class hkpWorld;
struct bsCoreCInfo;

class bsSceneGraph
{
	friend class bsSceneNode;

public:
	/*	Tree depth is the maximum node depth to be created. Total number of oct nodes may
		be as high as 8^n + 8^(n-1) + ... 8^1 + 8^0.
	*/
	bsSceneGraph(bsDx11Renderer* renderer, bsResourceManager* resourceManager,
		bsHavokManager* havokManager, const bsCoreCInfo& cInfo);

	~bsSceneGraph();

	bsSceneNode* createSceneNode(const hkVector4& position = hkVector4(0.0f, 0.0f, 0.0f, 0.0f));

	/**	Increments the amount of created objects and returns it.
		Used to assign unique IDs to objects.
	*/
	inline int getNewId()
	{
		return ++mNumCreatedNodes;
	}

	/**	Returns the amount of created objects without incrementing the value.
	*/
	inline int getNumCreatedNodes() const
	{
		return mNumCreatedNodes;
	}

	inline bsDx11Renderer* getRenderer() const 
	{
		return mDx11Renderer;
	}

	inline bsCamera* getCamera() const
	{
		return mCamera;
	}

	void update(float deltaTime);

	

private:
	bsCamera*	mCamera;

	std::vector<bsSceneNode*>	mSceneNodes;

	int					mNumCreatedNodes;
	bsDx11Renderer*		mDx11Renderer;
	bsResourceManager*	mResourceManager;
	unsigned short		mMaxTreeDepth;

	bsHavokManager*		mHavokManager;
	hkpWorld*			mGraphicsWorld;
};

#endif // BS_SCENE_GRAPH_H
