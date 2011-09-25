#include "StdAfx.h"

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
#include "bsRenderSystem.h"


bsCore::bsCore(const bsCoreCInfo& cInfo)
	: mCInfo(cInfo)
	, mRenderSystem(nullptr)
{
	bsLog::init(pantheios::SEV_DEBUG);

	BS_ASSERT2(mCInfo.isOk(), "Invalid construction info was sent to bsCore");

	bsLog::logMessage("Initializing core", pantheios::SEV_NOTICE);

	mWindow = new bsWindow(cInfo.windowWidth, cInfo.windowHeight, cInfo.windowName,
		cInfo.hInstance, cInfo.showCmd);

	mDx11Renderer = new bsDx11Renderer(mWindow->getHwnd(), cInfo.windowWidth, cInfo.windowHeight);

	mResourceManager = new bsResourceManager();
	mResourceManager->initAll(cInfo.assetDirectory, mDx11Renderer, mFileIoManager);

	mHavokManager = new bsHavokManager(cInfo.worldSize);

	mRenderQueue = new bsRenderQueue(mDx11Renderer, mResourceManager->getShaderManager());


	mFileIoThread = tbb::tbb_thread(std::bind(&bsFileIoManager::threadLoop, &mFileIoManager));
	bsWindowsUtils::setThreadName(GetThreadId(mFileIoThread.native_handle()), "Background File Loader");

	bsLog::logMessage("Initialization of core completed successfully", pantheios::SEV_NOTICE);
}

bsCore::~bsCore()
{
	//Shutdown file IO manager and join the thread it was using to avoid it trying to
	//access data that goes out of scope once this destructor returns.
	mFileIoManager.quit();
	mFileIoThread.join();

	delete mRenderQueue;

	delete mHavokManager;
	hkBaseSystem::quit();
	hkMemoryInitUtil::quit();

	delete mResourceManager;

	delete mDx11Renderer;

	delete mWindow;

	bsLog::logMessage("Core shut down successfully", pantheios::SEV_NOTICE);
	bsLog::deinit();
}

bool bsCore::update(float deltaTimeMs)
{
	BS_ASSERT2(mRenderSystem, "A render system must be set before calling bsCore::update");

	if (!mWindow->checkForMessages())
	{
		return false;
	}

	mRenderSystem->renderOneFrame();

	return true;
}
