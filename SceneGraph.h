#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <vector>
#include <memory>

#include "Node.h"
#include "OctNode.h"
#include "SceneNode.h"
#include "Dx11Renderer.h"
#include "Convert.h"

class ResourceManager;
class SceneNode;
class Camera;
class Dx11Renderer;


class SceneGraph
{
	friend class Node;
	friend class SceneNode;
	friend class OctNode;

public:
	SceneGraph();
	~SceneGraph();

	/*	Tree depth is the maximum node depth to be created. Total number of oct nodes may
		be as high as 8^n + 8^(n-1) + ... 8^1 + 8^0.
	*/
	void init(unsigned short treeDepth, Dx11Renderer* renderer, ResourceManager* resourceManager);

	SceneNode* createSceneNode(const hkVector4& position = hkVector4(0.0f, 0.0f, 0.0f, 0.0f));

	//Puts a scene node in the oct node it belongs in (the smallest one that contains it).
	//Leave depth at 1 if calling from an external class.
	void placeSceneNode(SceneNode* sceneNode, OctNode* octNode, unsigned short depth = 1);
	
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

	inline Dx11Renderer* getRenderer() const 
	{
		return mDx11Renderer;
	}

	inline OctNode* getRootNode() const
	{
		return mRootNode;
	}

	void drawAABBs(Dx11Renderer* dx11Renderer) const;

	const std::vector<std::shared_ptr<Renderable>> getVisibleRenderables() const;


	bool displayEmptyAabbs;

private:
	OctNode*	mRootNode;

	std::vector<SceneNode*>	mSceneNodes;
	std::vector<OctNode*>	mOctNodes;

	int					mNumCreatedObjects;
	Dx11Renderer*		mDx11Renderer;
	ResourceManager*	mResourceManager;
	Camera*				mCamera;
	unsigned short		mMaxTreeDepth;
};

#endif // SCENE_GRAPH_H