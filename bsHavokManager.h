#ifndef BS_HAVOKMANAGER_H
#define BS_HAVOKMANAGER_H

#include "bsConfig.h"

#include <Windows.h>

#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/System/Error/hkDefaultError.h>
#include <Common/Base/Memory/System/Util/hkMemoryInitUtil.h>
#include <Common/Base/Monitor/hkMonitorStream.h>
#include <Common/Base/Memory/System/hkMemorySystem.h>
#include <Common/Base/Memory/Allocator/Malloc/hkMallocAllocator.h>

#include <Physics/Collide/hkpCollide.h>
#include <Physics/Collide/Agent/ConvexAgent/SphereBox/hkpSphereBoxAgent.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics/Collide/Shape/Convex/Sphere/hkpSphereShape.h>
#include <Physics/Collide/Dispatch/hkpAgentRegisterUtil.h>

#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastInput.h>
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastOutput.h>

#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Utilities/Dynamics/Inertia/hkpInertiaTensorComputer.h>

#include <Common/Base/Thread/Job/ThreadPool/Cpu/hkCpuJobThreadPool.h>
#include <Common/Base/Thread/JobQueue/hkJobQueue.h>

#include <Common/Visualize/hkVisualDebugger.h>
#include <Physics/Utilities/VisualDebugger/hkpPhysicsContext.h>


/*	This class manages the core Havok objects, like the worlds, memory system, thread pool
	etc.
	Contains two hkpWorlds, one for visibility detection and one for normal physics.
*/
class bsHavokManager
{
public:
	bsHavokManager(const float worldSize);
	~bsHavokManager();

	/*	Creates the hkpWorld used for visibility detection etc.
	*/
	void createGraphicsWorld(bool createVisualDebugger = false);

	void createPhysicsWorld(bool createVisualDebugger = true);


	//Steps the Havok world for graphics with the provided time step.
	void stepGraphicsWorld(float deltaTimeMs);

	//Steps the Havok world for physics with the provided time step.
	void stepPhysicsWorld(float deltaTimeMs);

	inline hkpWorld* getGraphicsWorld() const
	{
		return mGraphicsWorld;
	}

	inline hkpWorld* getPhysicsWorld() const
	{
		return mPhysicsWorld;
	}
	

private:
	//Initializes things not directly related to the physics worlds.
	void createNonWorldObjects();

	void createVDB(hkpWorld* world);


	bool mNonWorldObjectsCreated;

	hkJobThreadPool*	mThreadPool;
	hkJobQueue*			mJobQueue;

	hkpWorld*			mPhysicsWorld;
	hkpWorld*			mGraphicsWorld;
	bool				mPhysicsVDBActive;
	bool				mGraphicsVDBActive;

	hkArray<hkProcessContext*>	mContexts;
	hkpPhysicsContext*			mContext;
	hkVisualDebugger*			mVisualDebugger;

	float		mWorldSize;
};

#endif // BS_HAVOKMANAGER_H
