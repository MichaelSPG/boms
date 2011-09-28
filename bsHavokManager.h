#pragma once


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
#include <Physics/Internal/BroadPhase/TreeBroadPhase/hkpTreeBroadPhase.h>

#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastInput.h>
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastOutput.h>

#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Utilities/Dynamics/Inertia/hkpInertiaTensorComputer.h>

#include <Common/Base/Thread/Job/ThreadPool/Cpu/hkCpuJobThreadPool.h>
#include <Common/Base/Thread/JobQueue/hkJobQueue.h>

#include <Common/Visualize/hkVisualDebugger.h>
#include <Physics/Utilities/VisualDebugger/hkpPhysicsContext.h>



/*	Simple class which forwards all messages sent by Havok to bsLog, with correct severity
	levels.
*/
class bsHavokErrorMessageRouter : public hkDefaultError
{
public:
	HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE);

	bsHavokErrorMessageRouter(hkErrorReportFunction errorReportFunction, void* errorReportObject = nullptr)
		: hkDefaultError(errorReportFunction, errorReportObject)
	{}


	/*	Convert the message type to bsLog severity level type and pass on the message to bsLog.
	*/
	virtual int message(Message m, int id, const char* description, const char* file,
		int line);
};

/*	This class manages the core Havok objects, like the worlds, memory system, thread pool
	etc.
*/
class bsHavokManager
{
public:
	bsHavokManager(const float worldSize);
	~bsHavokManager();

	void createWorld(bool createVisualDebugger = true);


	//Steps the Havok world for physics with the provided time step.
	void stepWorld(float deltaTimeMs);

	inline hkpWorld* getPhysicsWorld() const
	{
		return mWorld;
	}
	

private:
	//Initializes things not directly related to the physics worlds.
	void createNonWorldObjects();

	void createVDB(hkpWorld* world);


	bool mNonWorldObjectsCreated;

	hkJobThreadPool*	mThreadPool;
	hkJobQueue*			mJobQueue;

	hkpWorld*			mWorld;
	bool				mVDBActive;

	hkArray<hkProcessContext*>	mContexts;
	hkpPhysicsContext*			mContext;
	hkVisualDebugger*			mVisualDebugger;

	float		mWorldSize;
};
