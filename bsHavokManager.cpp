#include "StdAfx.h"

#include "bsHavokManager.h"

#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/System/Error/hkDefaultError.h>
#include <Common/Base/Memory/System/Util/hkMemoryInitUtil.h>
//#include <Common/Base/Monitor/hkMonitorStream.h>
#include <Common/Base/Memory/System/hkMemorySystem.h>
#include <Common/Base/Memory/Allocator/Malloc/hkMallocAllocator.h>

#include <Common/Base/Thread/Job/ThreadPool/Cpu/hkCpuJobThreadPool.h>

#include <Common/Visualize/hkVisualDebugger.h>
#include <Physics/Utilities/VisualDebugger/hkpPhysicsContext.h>

#include <Physics/Dynamics/World/hkpWorld.h>


#include <Common/Base/keycode.cxx>

#undef HK_FEATURE_PRODUCT_AI
#undef HK_FEATURE_PRODUCT_ANIMATION
#undef HK_FEATURE_PRODUCT_CLOTH
#undef HK_FEATURE_PRODUCT_DESTRUCTION
#undef HK_FEATURE_PRODUCT_BEHAVIOR

#define HK_EXCLUDE_LIBRARY_hkgpConvexDecomposition
#define HK_EXCLUDE_FEATURE_ConvexDecomposition

#define HK_FEATURE_REFLECTION_PHYSICS
#define HK_EXCLUDE_FEATURE_SerializeDeprecatedPre700
#define HK_EXCLUDE_FEATURE_RegisterVersionPatches
// Vdb needs the reflected classes
//#define HK_EXCLUDE_FEATURE_RegisterReflectedClasses
#define HK_EXCLUDE_FEATURE_MemoryTracker
#define HK_EXCLUDE_FEATURE_CompoundShape

//#define HK_EXCLUDE_LIBRARY_hkSceneData
#define HK_EXCLUDE_LIBRARY_hkVisualize
#define HK_EXCLUDE_LIBRARY_hkGeometryUtilities
#define HK_EXCLUDE_LIBRARY_hkCompat
#define HK_EXCLUDE_LIBRARY_hkcdInternal

//#define HK_EXCLUDE_LIBRARY_hkpUtilities
#define HK_EXCLUDE_FEATURE_hkpAabbTreeWorldManager
#define HK_EXCLUDE_FEATURE_hkpKdTreeWorldManager
#define HK_EXCLUDE_FEATURE_hkpVehicle
#define HK_EXCLUDE_FEATURE_hkpAccurateInertiaTensorComputer


#define HK_CLASSES_FILE <Common/Serialize/Classlist/hkKeyCodeClasses.h>

#include <Common/Base/Config/hkProductFeatures.cxx>

#include <string>

//#include <Windows.h>

#include "bsLog.h"
#include "bsAssert.h"


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
		int line)
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
};

/*
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
}
*/

bsHavokManager::bsHavokManager()
	: mContext(nullptr)
	, mVisualDebugger(nullptr)
{
	//Allocate 2.5 MB for physics solver.
	hkMemoryRouter* memoryRouter = hkMemoryInitUtil::initDefault(
		hkMallocAllocator::m_defaultMallocAllocator, hkMemorySystem::FrameInfo(2500000));
	//hkBaseSystem::init(memoryRouter, errorReport);
	hkBaseSystem::init(memoryRouter, nullptr);

	//Replace default error instance to reroute messages to the log with correct severity
	//levels.
	//hkError::replaceInstance(new bsHavokErrorMessageRouter(errorReport, nullptr));
	hkError::replaceInstance(new bsHavokErrorMessageRouter(nullptr, nullptr));

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
	//hkError::getInstance().setEnabled(0x2A2CDE91, false);

	//Phantom queried for collidables without call to ensureDeterministicOrder().
	//The result is potentially nondeterministic.
	//hkError::getInstance().setEnabled(0x5FE59C18, false);
}

bsHavokManager::~bsHavokManager()
{
	BS_ASSERT2(mVisualDebugger == nullptr, "Visual debugger has been created, but has not"
		" been destroyed. Did you forget to call destroyVisualDebuggerForWorld()?");
	
	if (mJobQueue)
	{
		delete mJobQueue;
		mThreadPool->removeReference();
	}
	
	/*	hkMemoryInitUtil::quit() and hkBaseSystem::quit() must be called elsewhere
		because mContexts will go out of scope here, which needs them to deallocate.
	*/
}

void bsHavokManager::createVisualDebuggerForWorld(hkpWorld& world)
{
	BS_ASSERT2(!mVisualDebugger, "Visual debugger has already been created");

	//Create visual debugger and start listening for connections.
	mContext = new hkpPhysicsContext();
	hkpPhysicsContext::registerAllPhysicsProcesses();
	mContext->addWorld(&world);
	mContexts.pushBack(mContext);
	
	mVisualDebugger = new hkVisualDebugger(mContexts);
	mVisualDebugger->serve();
}

void bsHavokManager::destroyVisualDebuggerForWorld(hkpWorld& world)
{
	world.markForWrite();

	mVisualDebugger->removeReference();
	mContext->removeReference();
	mVisualDebugger = nullptr;

	world.unmarkForWrite();
}

void bsHavokManager::stepWorld(hkpWorld& world, float deltaTimeMs)
{
	const hkpStepResult result
		//= mWorld->stepMultithreaded(mJobQueue, mThreadPool, deltaTimeMs * 0.001f);
		= world.stepMultithreaded(mJobQueue, mThreadPool, 0.016666667f);

	BS_ASSERT2(result == HK_STEP_RESULT_SUCCESS, "Failed to step world");
}

void bsHavokManager::stepVisualDebugger(float deltaTimeMs)
{
	BS_ASSERT2(mVisualDebugger, "Trying to step visual debugger, but it has not been created");

	mContext->syncTimers(mThreadPool);
	//mVisualDebugger->step(deltaTimeMs);
	mVisualDebugger->step(0.016666667f);

	hkMonitorStream::getInstance().reset();
	mThreadPool->clearTimerData();
}
