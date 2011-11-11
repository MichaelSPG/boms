#include "StdAfx.h"

#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/Memory/System/Util/hkMemoryInitUtil.h>

#include "bsCore.h"
#include "bsWindow.h"
#include "bsWindowsUtils.h"
#include "bsScene.h"
#include "bsDx11Renderer.h"
#include "bsResourceManager.h"
#include "bsHavokManager.h"
#include "bsLog.h"
#include "bsAssert.h"
#include "bsRenderQueue.h"
#include "bsDeferredRenderer.h"
#include "bsTimer.h"
#include "bsFrameStatistics.h"


bsCore::bsCore(const bsCoreCInfo& cInfo)
	: mCInfo(cInfo)
	, mResizeQueued(false)
	, mResizeWidth(0)
	, mResizeHeight(0)
{
	bsLog::init(fopen(cInfo.logFileFileName.c_str(), "w"), bsLog::SEV_DEBUG,
		bsLog::TIMESTAMP_MILLISECS | bsLog::SEVERITY_AS_TEXT);

	BS_ASSERT2(mCInfo.isOk(), "Invalid construction info was sent to bsCore");

	bsLog::log("Initializing core");

	auto windowResizeCallback = std::bind(&bsCore::windowResizedCallback,
		this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3);

	mWindow = new bsWindow(cInfo.windowWidth, cInfo.windowHeight, cInfo.windowName,
		cInfo.hInstance, cInfo.showCmd, windowResizeCallback);

	mDx11Renderer = new bsDx11Renderer(mWindow->getHwnd(), cInfo.windowWidth, cInfo.windowHeight);

	mResourceManager = new bsResourceManager();
	mResourceManager->initAll(cInfo.assetDirectory, mDx11Renderer, mFileIoManager);

	mHavokManager = new bsHavokManager();

	mRenderQueue = new bsRenderQueue(mDx11Renderer, mResourceManager->getShaderManager());

	mRenderSystem = new bsDeferredRenderer(mDx11Renderer,
		mResourceManager->getShaderManager(), mWindow, mRenderQueue);

	mFileIoThread = new tbb::tbb_thread(std::bind(&bsFileIoManager::threadLoop, &mFileIoManager));
	bsWindowsUtils::setThreadName(GetThreadId(mFileIoThread->native_handle()), "Background File Loader");

	bsLog::log("Initialization of core completed successfully");
}

bsCore::~bsCore()
{
	//This avoids reference counting problems where a log callback is hooked to a text
	//box or similar.
	bsLog::clearAllCallbacks();

	//Shutdown file IO manager and join the thread it was using to avoid it trying to
	//access data that goes out of scope once this destructor returns.
	mFileIoManager.quit();
	mFileIoThread->join();
	delete mFileIoThread;

	delete mRenderSystem;
	delete mRenderQueue;

	delete mHavokManager;
	hkBaseSystem::quit();
	hkMemoryInitUtil::quit();

	delete mResourceManager;

	delete mDx11Renderer;

	delete mWindow;

	bsLog::log("Core shut down successfully, closing log");
	bsLog::deinit();
}

bool bsCore::update(float deltaTimeMs, bsFrameStatistics& framStatistics)
{
	if (!mWindow->checkForMessages())
	{
		return false;
	}

	if (mResizeQueued)
	{
		mDx11Renderer->resizeWindow(mWindow->getHwnd(), mResizeWidth, mResizeHeight);

		mResizeQueued = false;
	}

	bsTimer timer;
	float preRender = timer.getTimeMilliSeconds();

	mRenderSystem->renderOneFrame(framStatistics);

	framStatistics.renderingInfo.totalRenderingDuration = timer.getTimeMilliSeconds() - preRender;

	return true;
}

void bsCore::windowResizedCallback(unsigned int width, unsigned int height,
	const bsWindow& window)
{
	mResizeQueued = true;
	mResizeWidth = width;
	mResizeHeight = height;
}
