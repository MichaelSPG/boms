#pragma once

#include <Common/Base/hkBase.h>
#include <Common/Base/Container/Array/hkArray.h>

class hkJobThreadPool;
class hkJobQueue;
class hkProcessContext;
class hkpPhysicsContext;
class hkVisualDebugger;
class hkpWorld;


/*	This class manages the core Havok objects, like the memory system, thread pool, etc.
	It is also responsible for multi-threaded stepping of physics worlds.
*/
class bsHavokManager
{
public:
	bsHavokManager();
	~bsHavokManager();

	void createVisualDebuggerForWorld(hkpWorld& world);

	void destroyVisualDebuggerForWorld(hkpWorld& world);

	//Steps the Havok world for physics with the provided time step.
	void stepWorld(hkpWorld& world, float deltaTimeMs);

	void stepVisualDebugger(float deltaTimeMs);

	inline hkJobQueue& getJobQueue()
	{
		return *mJobQueue;
	}

	/*	Returns information about memory used by Havok.
	*/
	hkMemoryAllocator::MemoryStatistics getMemoryStatistics() const;

private:
	void createVDB(hkpWorld* world);


	hkJobThreadPool*	mThreadPool;
	hkJobQueue*			mJobQueue;

	hkArray<hkProcessContext*>	mContexts;
	hkpPhysicsContext*			mContext;
	hkVisualDebugger*			mVisualDebugger;
};
