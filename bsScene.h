#pragma once

#include <vector>
#include <memory>

#include <Common/Base/hkBase.h>

class bsCamera;
class bsDx11Renderer;
class bsRenderable;
class bsHavokManager;
class hkpWorld;
struct bsCoreCInfo;
class hkJobQueue;
class bsEntity;


/*	Keys used for properties attached to Havok rigid bodies and similar, making it
	possible to find an entity from a Havok rigid body.
*/
enum bsPropertyKeys
{
	//Pointer to the bsEntity owning the Havok entity.
	BSPK_ENTITY_POINTER = 10000
};


/*	A scene represents a collection of entities.
	A scene must always contain at least one camera (created by default).

	All the entities in a scene is owned by that scene, and upon destruction of a scene,
	all of the entities in it will also be destroyed. If you do not want specific entities
	to be destroyed when the scene is destroyed, remove them before destroying the scene
	by calling removeEntity().
*/
class bsScene
{
public:
	bsScene(bsDx11Renderer* renderer, bsHavokManager* havokManager,
		const bsCoreCInfo& cInfo);

	~bsScene();


	/*	Adds an entity to the scene, and to the physics world if it contains physics
		components.
	*/
	void addEntity(bsEntity& entity);

	/*	Removes an entity from the scene, and from the physical world if it contains
		physics components.
	*/
	void removeEntity(bsEntity& entityToRemove);

	inline bsDx11Renderer* getRenderer() const 
	{
		return mDx11Renderer;
	}

	inline bsCamera* getCamera() const
	{
		return mCamera;
	}

	/*	Returns a vector containing every entity currently in the scene.
		
	*/
	inline const std::vector<bsEntity*>& getEntities() const
	{
		return mEntities;
	}
	
	inline hkpWorld* getPhysicsWorld() const
	{
		return mPhysicsWorld;
	}

	void update(float deltaTimeMs);


	/*	Enabled or disables stepping of physics. This can be used to closely inspect the
		state of the world during a frame without the physics affecting objects, making
		it easier to take screenshots or similar of explosions, etc.
	*/
	inline void setStepPhysicsEnabled(bool stepPhysics)
	{
		mStepPhysics = stepPhysics;
	}

	inline bool isStepPhysicsEnabled() const
	{
		return mStepPhysics;
	}

private:
	/*	Increments the amount of created entities and returns it.
		Used to assign unique IDs to entities.
	*/
	inline unsigned int getNewId()
	{
		return ++mNumCreatedEntities;
	}

	/*	Creates a Havok world.
	*/
	void createPhysicsWorld(hkJobQueue& jobQueue);

	/*	Synchronizes all active (non-sleeping) rigid bodies with their entities.
	*/
	void synchronizeActiveEntities();


	bsCamera*	mCamera;

	std::vector<bsEntity*>	mEntities;

	unsigned int		mNumCreatedEntities;
	bsDx11Renderer*		mDx11Renderer;

	hkpWorld*	mPhysicsWorld;

	bsHavokManager* mHavokManager;

	//Whether physics should be stepped.
	bool mStepPhysics;
};
