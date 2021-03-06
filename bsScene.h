#pragma once

#include <vector>

#include <Common/Base/hkBase.h>

#include "bsContactCounter.h"

class bsCamera;
class bsDx11Renderer;
class bsRenderable;
class bsHavokManager;
class hkpWorld;
struct bsCoreCInfo;
struct bsFrameStatistics;
class hkJobQueue;
class bsEntity;


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

	void update(float deltaTimeMs, bsFrameStatistics& framStatistics);


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

	/*	Sets the time scale of the physics world.
		Making sudden non-small changes to this value is not encouraged as it will reduce the
		stability of the simulation. Larger values (over 1) will also reduce the stability.
		Default is 1.
	*/
	inline void setTimeScale(float newTimeScale)
	{
		mTimeScale = newTimeScale;
	}

	inline float getTimeScale() const
	{
		return mTimeScale;
	}

	/*	Sets the amount of physics steps to perform per second, and the size of each
		step in ms. The step size will set to 1000 / stepsPerSecond.
		Making sudden non-small changes to this value is not encouraged as it will reduce the
		stability of the simulation.
	*/
	void setPhysicsFrequency(float stepsPerSecond)
	{
		mPhysicsFrequency = stepsPerSecond;
		mPhysicsStepSizeMs = 1000.0f / stepsPerSecond;
	}

	float getPhysicsFrequency() const
	{
		return mPhysicsFrequency;
	}


private:
	/*	This recursively removes entityToRemove and all of its children, all of the
		children's children, and so on, from the scene.
		This function can also delete all the entities it removes.
	*/
	void removeEntityAndChildrenRecursively(bsEntity& entityToRemove, bool deleteAfterRemoving);


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
		The two parameters are output parameters and will contain information about the
		current state of the physics simulation.
	*/
	void synchronizeActiveEntities(unsigned int* totalActiveRigidBodies,
		unsigned int* totalActiveSimulationIslands);


	bsCamera*	mCamera;

	std::vector<bsEntity*>	mEntities;

	unsigned int		mNumCreatedEntities;
	bsDx11Renderer*		mDx11Renderer;

	hkpWorld*			mPhysicsWorld;
	bsContactCounter	mContactCounter;

	bsHavokManager* mHavokManager;

	//Whether physics should be stepped.
	bool mStepPhysics;

	float				mTimeScale;
	//Number of physics steps to perform per second.
	float				mPhysicsFrequency;
	//Size of each physics step, in milliseconds.
	float				mPhysicsStepSizeMs;
};
