#include "StdAfx.h"

#include "bsHavokManager.h"

#include <Common/Base/keycode.cxx>

#undef HK_FEATURE_PRODUCT_AI
#undef HK_FEATURE_PRODUCT_ANIMATION
#undef HK_FEATURE_PRODUCT_CLOTH
#undef HK_FEATURE_PRODUCT_DESTRUCTION
#undef HK_FEATURE_PRODUCT_BEHAVIOR

#define HK_EXCLUDE_LIBRARY_hkgpConvexDecomposition

#define HK_FEATURE_REFLECTION_PHYSICS
#define HK_EXCLUDE_FEATURE_SerializeDeprecatedPre700
#define HK_EXCLUDE_FEATURE_RegisterVersionPatches
// Vdb needs the reflected classes
//#define HK_EXCLUDE_FEATURE_RegisterReflectedClasses
//#define HK_EXCLUDE_FEATURE_MemoryTracker
//#define HK_EXCLUDE_FEATURE_hkpAccurateInertiaTensorComputer
//#define HK_EXCLUDE_FEATURE_CompoundShape
#define HK_EXCLUDE_FEATURE_hkpAabbTreeWorldManager
#define HK_EXCLUDE_FEATURE_hkpKdTreeWorldManager
#define HK_EXCLUDE_FEATURE_hkpVehicle
#define HK_EXCLUDE_FEATURE_hkpAccurateInertiaTensorComputer

#define HK_CLASSES_FILE <Common/Serialize/Classlist/hkKeyCodeClasses.h>

#include <Common/Base/Config/hkProductFeatures.cxx>

#include <string>

#include <Windows.h>

#include "bsLog.h"
#include "bsAssert.h"


int bsHavokErrorMessageRouter::message( Message m, int id, const char* description, const char* file, int line )
{
	//Havok likes to end its messages with \n, so we remove that here before sending it
	//to the logger.
	std::string correctedMessage("Havok: ");
	correctedMessage.append(description);

	while (correctedMessage.back() == '\n')
	{
		correctedMessage.pop_back();
	}


	switch (m)
	{
	case MESSAGE_ERROR:
	case MESSAGE_ASSERT:
		bsLog::logMessage(correctedMessage.c_str(), bsLog::SEV_ERROR);
		return 1;

	case MESSAGE_WARNING:
		bsLog::logMessage(correctedMessage.c_str(), bsLog::SEV_WARNING);
		break;

	case MESSAGE_REPORT:
		bsLog::logMessage(correctedMessage.c_str(), bsLog::SEV_NOTICE);
		break;
	}

	return 0;
}


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
	bsLog::logMessage(correctedMessage.c_str(), bsLog::SEV_WARNING);

#ifdef BS_DEBUG
	if (IsDebuggerPresent() != 0)
	{
		correctedMessage.push_back('\n');
		OutputDebugStringA(correctedMessage.c_str());
	}
#endif
}


bsHavokManager::bsHavokManager(const float worldSize)
	: mNonWorldObjectsCreated(false)
	, mThreadPool(nullptr)
	, mJobQueue(nullptr)
	, mWorld(nullptr)
	, mVDBActive(false)
	, mContext(nullptr)
	, mVisualDebugger(nullptr)
	, mWorldSize(worldSize)
{
}

bsHavokManager::~bsHavokManager()
{
	if (mWorld)
	{
		mWorld->markForWrite();
		mWorld->removeReference();
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

void bsHavokManager::createWorld(bool createVisualDebugger /*= true*/)
{
	if (!mNonWorldObjectsCreated)
	{
		createNonWorldObjects();
	}

	hkpWorldCinfo worldCinfo;
	worldCinfo.setBroadPhaseWorldSize(mWorldSize);
	worldCinfo.m_solverIterations;
	worldCinfo.m_enableDeactivation;
	worldCinfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_MULTITHREADED;
	worldCinfo.m_broadPhaseBorderBehaviour = hkpWorldCinfo::BROADPHASE_BORDER_REMOVE_ENTITY;

	mWorld = new hkpWorld(worldCinfo);
	mWorld->markForWrite();

	hkpAgentRegisterUtil::registerAllAgents(mWorld->getCollisionDispatcher());
	mWorld->registerWithJobQueue(mJobQueue);

	mWorld->unmarkForWrite();

	if (createVisualDebugger)
	{
		BS_ASSERT(mVisualDebugger == nullptr);
		createVDB(mWorld);
		mVDBActive = true;
	}
}

void bsHavokManager::createNonWorldObjects()
{
	//Allocate 2.5 MB for physics solver.
	hkMemoryRouter* memoryRouter = hkMemoryInitUtil::initDefault(
		hkMallocAllocator::m_defaultMallocAllocator, hkMemorySystem::FrameInfo(2500000));
	hkBaseSystem::init(memoryRouter, errorReport);

	//Replace default error instance to reroute messages to the log with correct severity
	//levels.
	hkError::replaceInstance(new bsHavokErrorMessageRouter(errorReport, nullptr));

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
	BS_ASSERT2(!mVisualDebugger, "Visual debugger has already been created");

	//Create visual debugger and start listening for connections.
	mContext = new hkpPhysicsContext();
	hkpPhysicsContext::registerAllPhysicsProcesses();
	mContext->addWorld(world);
	mContexts.pushBack(mContext);
	
	mVisualDebugger = new hkVisualDebugger(mContexts);
	mVisualDebugger->serve();

	mVDBActive = true;
}

void bsHavokManager::stepWorld(float deltaTimeMs)
{
	BS_ASSERT2(mWorld, "Physics world is being stepped before it has been created");

	const hkpStepResult result
		//= mWorld->stepMultithreaded(mJobQueue, mThreadPool, deltaTimeMs * 0.001f);
		= mWorld->stepMultithreaded(mJobQueue, mThreadPool, 0.016666667f);

	BS_ASSERT2(result == HK_STEP_RESULT_SUCCESS, "Failed to step world");
	//step world

	if (mVDBActive)
	{
		mContext->syncTimers(mThreadPool);
		mVisualDebugger->step(deltaTimeMs);

		hkMonitorStream::getInstance().reset();
		mThreadPool->clearTimerData();
	}
}
