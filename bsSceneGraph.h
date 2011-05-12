#ifndef BS_SCENE_GRAPH_H
#define BS_SCENE_GRAPH_H

#include <vector>
#include <memory>

#include <Common/Base/hkBase.h>

class bsResourceManager;
class bsSceneNode;
class bsCamera;
class bsDx11Renderer;
class bsRenderable;
class bsHavokManager;
class hkpWorld;
struct bsCoreCInfo;


/*	The scene graph is responsibe for creation of scene nodes.
	It also creates the Havok world used for visibility detection of nodes.
*/
class bsSceneGraph
{
	friend class bsSceneNode;

public:
	bsSceneGraph(bsDx11Renderer* renderer, bsResourceManager* resourceManager,
		bsHavokManager* havokManager, const bsCoreCInfo& cInfo);

	~bsSceneGraph();

	/*	Creates a new scene node with the given translation.
		This new node will not be the child of any other nodes.
	*/
	bsSceneNode* createSceneNode(const hkVector4& position = hkVector4(0.0f, 0.0f, 0.0f, 0.0f));

	/*	Returns the amount of created nodes.
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
	/*	Increments the amount of created nodes and returns it.
		Used to assign unique IDs to nodes.
	*/
	inline int getNewId()
	{
		return ++mNumCreatedNodes;
	}


	bsCamera*	mCamera;

	std::vector<bsSceneNode*>	mSceneNodes;

	int					mNumCreatedNodes;
	bsDx11Renderer*		mDx11Renderer;
	bsResourceManager*	mResourceManager;
	unsigned short		mMaxTreeDepth;

	hkpWorld*			mGraphicsWorld;
};

#endif // BS_SCENE_GRAPH_H
