#include "bsHavokManager.h"

#include <Common/Base/keycode.cxx>

#undef HK_FEATURE_PRODUCT_AI
#undef HK_FEATURE_PRODUCT_ANIMATION
#undef HK_FEATURE_PRODUCT_CLOTH
#undef HK_FEATURE_PRODUCT_DESTRUCTION
#undef HK_FEATURE_PRODUCT_BEHAVIOR
#define HK_EXCLUDE_LIBRARY_hkgpConvexDecomposition
#define HK_FEATURE_REFLECTION_PHYSICS
#define HK_CLASSES_FILE <Common/Serialize/Classlist/hkKeyCodeClasses.h>

#include <Common/Base/Config/hkProductFeatures.cxx>
#include <string>
#include <cassert>

#include "bsLog.h"


static void HK_CALL errorReport(const char* message, void* userArgGivenToInit)
{
	//Havok likes to end its messages with \n, so we remove that here before sending it
	//to the log.
	std::string correctedMessage("Havok: ");
	correctedMessage.append(message);

	while (correctedMessage.back() == '\n')
	{
		correctedMessage.pop_back();
	}
	bsLog::logMessage(correctedMessage.c_str(), pantheios::SEV_WARNING);
}


bsHavokManager::bsHavokManager(const float worldSize)
	: mNonWorldObjectsCreated(false)
	, mThreadPool(nullptr)
	, mJobQueue(nullptr)
	, mPhysicsWorld(nullptr)
	, mGraphicsWorld(nullptr)
	, mPhysicsVDBActive(false)
	, mGraphicsVDBActive(false)
	, mContext(nullptr)
	, mVisualDebugger(nullptr)
	, mWorldSize(worldSize)
{
}

bsHavokManager::~bsHavokManager()
{
	if (mGraphicsWorld)
	{
		mGraphicsWorld->removeReference();
	}
	if (mPhysicsWorld)
	{
		mPhysicsWorld->removeReference();
	}
	if (mVisualDebugger)
	{
		mVisualDebugger->removeReference();
		mContext->removeReference();
	}
	if (mJobQueue)
	{
		delete mJobQueue;
		mThreadPool->removeReference();
	}
	
	/*	hkMemoryInitUtil::quit() and hkBaseSystem::quit() must be called elsewhere
		because mContexts will go out of scope here, which needs them to deallocate.
	*/
}

void bsHavokManager::createGraphicsWorld(bool createVisualDebugger /*= false*/)
{
	if (mGraphicsWorld)
	{
		bsLog::logMessage("Graphics world already exists", pantheios::SEV_CRITICAL);
		return;
	}
	assert(!mGraphicsWorld && "Graphics world already exists");

	if (!mNonWorldObjectsCreated)
	{
		createNonWorldObjects();
	}

	hkpWorldCinfo worldCinfo;
	worldCinfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_DISCRETE;
	worldCinfo.m_broadPhaseBorderBehaviour = hkpWorldCinfo::BROADPHASE_BORDER_ASSERT;
	worldCinfo.m_gravity.setAll3(0.0f);
	worldCinfo.m_solverIterations = 1;
	worldCinfo.m_solverDamp = 1.0f;
	worldCinfo.m_solverTau = 1.0f;
	worldCinfo.setBroadPhaseWorldSize(mWorldSize * 12.0f);

	mGraphicsWorld = new hkpWorld(worldCinfo);
	hkpAgentRegisterUtil::registerAllAgents(mGraphicsWorld->getCollisionDispatcher());

	if (createVisualDebugger)
	{
		createVDB(mGraphicsWorld);
		mGraphicsVDBActive = true;
	}
}

void bsHavokManager::createPhysicsWorld(bool createVisualDebugger /*= true*/)
{
	if (!mNonWorldObjectsCreated)
	{
		createNonWorldObjects();
	}

	assert(!"Not implemented");


	if (createVisualDebugger)
	{
		createVDB(mPhysicsWorld);
		mPhysicsVDBActive = true;
	}
}

void bsHavokManager::createNonWorldObjects()
{
	//Allocate 0.5 MB for physics solver
	hkMemoryRouter* memoryRouter = hkMemoryInitUtil::initDefault(
		hkMallocAllocator::m_defaultMallocAllocator, hkMemorySystem::FrameInfo(500000));
	hkBaseSystem::init(memoryRouter, errorReport);

	//Get number of available physical threads
	hkHardwareInfo hwInfo;
	hkGetHardwareInfo(hwInfo);
	int totalNumThreadsUsed = hwInfo.m_numThreads;

	hkCpuJobThreadPoolCinfo threadPoolCinfo;
	threadPoolCinfo.m_numThreads = totalNumThreadsUsed - 1;
	threadPoolCinfo.m_timerBufferPerThreadAllocation = 2000000;//2 MB
	mThreadPool = new hkCpuJobThreadPool(threadPoolCinfo);

	hkJobQueueCinfo jobQueueInfo;
	jobQueueInfo.m_jobQueueHwSetup.m_numCpuThreads = totalNumThreadsUsed;
	mJobQueue = new hkJobQueue(jobQueueInfo);

	//For VDB timers
	hkMonitorStream::getInstance().resize(2000000);

	//Simulation may become unstable, the time step has decreased by more than a factor
	//of 4.000000 from the previous step
	hkError::getInstance().setEnabled(0x2A2CDE91, false);

	//Phantom queried for collidables without call to ensureDeterministicOrder().
	//The result is potentially nondeterministic.
	hkError::getInstance().setEnabled(0x5FE59C18, false);

	mNonWorldObjectsCreated = true;
}

void bsHavokManager::createVDB(hkpWorld* world)
{
	if (mVisualDebugger)
	{
		bsLog::logMessage("Visual debugger has already been created",
			pantheios::SEV_ERROR);

		return;
	}
	assert(!mVisualDebugger && "Visual debugger has already been created");

	//Create visual debugger and start listening for connections.
	mContext = new hkpPhysicsContext();
	hkpPhysicsContext::registerAllPhysicsProcesses();
	mContext->addWorld(world);
	mContexts.pushBack(mContext);
	
	mVisualDebugger = new hkVisualDebugger(mContexts);
	mVisualDebugger->serve();
}

void bsHavokManager::stepGraphicsWorld(float deltaTimeMs)
{
	assert(mGraphicsWorld);

	//Convert to seconds since Havok uses seconds.
	mGraphicsWorld->stepDeltaTime(deltaTimeMs * 0.001f);

	if (mGraphicsVDBActive)
	{
		//mContext->syncTimers();
		mVisualDebugger->step(deltaTimeMs);
	}
}

void bsHavokManager::stepPhysicsWorld(float deltaTimeMs)
{
	assert(mPhysicsWorld);
	assert(!"stepPhysicsWorld not implemented");

	//step world

	if (mPhysicsVDBActive)
	{
		mContext->syncTimers(mThreadPool);
		mVisualDebugger->step(deltaTimeMs);

		hkMonitorStream::getInstance().reset();
		mThreadPool->clearTimerData();
	}
}
