#ifndef BS_SCENE_GRAPH_H
#define BS_SCENE_GRAPH_H

#include <vector>
#include <memory>

#include "bsNode.h"
#include "bsOctNode.h"
#include "bsSceneNode.h"
#include "bsDx11Renderer.h"
#include "bsConvert.h"

class bsResourceManager;
class bsSceneNode;
class bsCamera;
class bsDx11Renderer;


class bsSceneGraph
{
	friend class bsNode;
	friend class bsSceneNode;
	friend class bsOctNode;

public:
	/*	Tree depth is the maximum node depth to be created. Total number of oct nodes may
		be as high as 8^n + 8^(n-1) + ... 8^1 + 8^0.
	*/
	bsSceneGraph(unsigned short treeDepth, bsDx11Renderer* renderer,
		bsResourceManager* resourceManager);

	~bsSceneGraph();

	bsSceneNode* createSceneNode(const hkVector4& position = hkVector4(0.0f, 0.0f, 0.0f, 0.0f));

	//Puts a scene node in the oct node it belongs in (the smallest one that contains it).
	//Leave depth at 1 if calling from an external class.
	void placeSceneNode(bsSceneNode* sceneNode, bsOctNode* octNode, unsigned short depth = 1);
	
	/**	Increments the amount of created objects and returns it.
		Used to assign unique IDs to objects.
	*/
	inline const int getNumCreatedObjects()
	{
		return ++mNumCreatedObjects;
	}

	/**	Returns the amount of created objects without incrementing the value.
	*/
	inline const int getNumCreatedObjectsNoIncrement() const
	{
		return mNumCreatedObjects;
	}

	inline bsDx11Renderer* getRenderer() const 
	{
		return mDx11Renderer;
	}

	inline bsOctNode* getRootNode() const
	{
		return mRootNode;
	}

	void drawAABBs(bsDx11Renderer* dx11Renderer) const;

	const std::vector<std::shared_ptr<bsRenderable>> getVisibleRenderables() const;


	bool displayEmptyAabbs;

private:
	bsOctNode*	mRootNode;

	std::vector<bsSceneNode*>	mSceneNodes;
	std::vector<bsOctNode*>	mOctNodes;

	int					mNumCreatedObjects;
	bsDx11Renderer*		mDx11Renderer;
	bsResourceManager*	mResourceManager;
	bsCamera*				mCamera;
	unsigned short		mMaxTreeDepth;
};

#endif // BS_SCENE_GRAPH_H
