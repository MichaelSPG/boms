#pragma once

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


//Keys used for properties attached to Havok rigid bodies and similar.
enum bsPropertyKeys
{
	//Pointer to the bsEntity owning the Havok entity.
	BSPK_ENTITY_POINTER = 10000
};


/*	The scene graph is responsibe for creation of scene nodes.
	It also creates the Havok world used for visibility detection of nodes.
*/
class bsScene
{
	friend class bsSceneNode;

public:
	bsScene(bsDx11Renderer* renderer, bsResourceManager* resourceManager,
		bsHavokManager* havokManager, const bsCoreCInfo& cInfo);

	~bsScene();

	/*	Creates a new scene node with the given translation.
		This new node will not be the child of any other nodes.
	*/
	//bsSceneNode* createSceneNode(const hkVector4& position = hkVector4(0.0f, 0.0f, 0.0f, 0.0f));

	/*	Adds a scene node to the scene, and to the physics world if it contains physics components.
	*/
	void addSceneNode(bsSceneNode* sceneNode);

	inline bsDx11Renderer* getRenderer() const 
	{
		return mDx11Renderer;
	}

	inline bsCamera* getCamera() const
	{
		return mCamera;
	}

	inline const std::vector<bsSceneNode*> getSceneNodes() const
	{
		return mSceneNodes;
	}
	
	inline hkpWorld* getPhysicsWorld() const
	{
		return mPhysicsWorld;
	}

	void update(float deltaTime);

private:
	/*	Increments the amount of created nodes and returns it.
		Used to assign unique IDs to nodes.
	*/
	inline unsigned int getNewId()
	{
		return ++mNumCreatedNodes;
	}

	/*	Synchronizes all active (non-sleeping) rigid bodies with their entities.
	*/
	void synchronizeActiveEntities();


	bsCamera*	mCamera;

	std::vector<bsSceneNode*>	mSceneNodes;

	unsigned int					mNumCreatedNodes;
	bsDx11Renderer*		mDx11Renderer;
	bsResourceManager*	mResourceManager;

	hkpWorld*	mGraphicsWorld;
	hkpWorld*	mPhysicsWorld;

	bsHavokManager* mHavokManager;
};
