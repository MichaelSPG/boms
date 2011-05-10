#include "bsCore.h"

#include <cassert>

#include "bsWindow.h"
#include "bsSceneGraph.h"
#include "bsDx11Renderer.h"
#include "bsResourceManager.h"
#include "bsHavokManager.h"
#include "bsLog.h"
#include "bsRenderQueue.h"
#include "bsRenderSystem.h"


bsCore::bsCore(const bsCoreCInfo& cInfo)
	: mCInfo(cInfo)
{
	bsLog::init(pantheios::SEV_DEBUG);
	if (!mCInfo.isOk())
	{
		bsLog::logMessage("Invalid construction info was sent to bsCore",
			pantheios::SEV_EMERGENCY);
		
		assert(mCInfo.isOk());
		return;
	}

	bsLog::logMessage("Initializing core", pantheios::SEV_NOTICE);

	mWindow = new bsWindow(cInfo.windowWidth, cInfo.windowHeight, cInfo.windowName,
		cInfo.hInstance, cInfo.showCmd);

	mDx11Renderer = new bsDx11Renderer(mWindow->getHwnd(), cInfo.windowWidth, cInfo.windowHeight);

	mResourceManager = new bsResourceManager();
	mResourceManager->initAll(cInfo.assetDirectory, mDx11Renderer);

	mHavokManager = new bsHavokManager(cInfo.worldSize);

	mSceneGraph = new bsSceneGraph(mDx11Renderer, mResourceManager, mHavokManager, cInfo);

	mRenderQueue = new bsRenderQueue(mDx11Renderer, mResourceManager->getShaderManager());
	mRenderQueue->setCamera(mSceneGraph->getCamera());

	bsLog::logMessage("Initialization of core completed successfully", pantheios::SEV_NOTICE);
}

bsCore::~bsCore()
{
	delete mRenderQueue;

	delete mSceneGraph;

	delete mHavokManager;
	hkMemoryInitUtil::quit();
	hkBaseSystem::quit();

	delete mResourceManager;

	delete mDx11Renderer;

	delete mWindow;
}

bool bsCore::update(float deltaTimeMs)
{
	if (!mWindow->checkForMessages())
	{
		return false;
	}

	mHavokManager->stepGraphicsWorld(deltaTimeMs);
	//mHavokManager->stepPhysicsWorld(deltaTimeMs);

	mRenderSystem->renderOneFrame();

	//mDx11Renderer->clearBackBuffer();
	//mDx11Renderer->clearRenderTargets()

	//mRenderQueue->reset();
	//mRenderQueue->draw();

	//mResourceManager->getTextManager()->drawAllTexts(deltaTimeMs);

	//mDx11Renderer->present();

	return true;
}
