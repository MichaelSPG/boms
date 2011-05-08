#include "bsResourceManager.h"

#include <cassert>

#include "bsLog.h"


//TODO: Replace debug checks with asserts.

bsResourceManager::bsResourceManager()
	: mFileSystem(nullptr)
	, mShaderManager(nullptr)
	, mMeshManager(nullptr)
	, mTextManager(nullptr)
{
}

bsResourceManager::~bsResourceManager()
{
	if (mFileSystem)
	{
		delete mFileSystem;
	}
	if (mShaderManager)
	{
		delete mShaderManager;
	}
	if (mMeshManager)
	{
		delete mMeshManager;
	}
	if (mTextManager)
	{
		delete mTextManager;
	}
}

void bsResourceManager::initAll(const std::string& fileSystemBasePath,
	bsDx11Renderer* dx11Renderer)
{
	initFileSystem(fileSystemBasePath);
	initShaderManager(dx11Renderer);
	initMeshManager(dx11Renderer);
	initTextManager(dx11Renderer);
}

void bsResourceManager::initFileSystem(const std::string& basePath)
{
	assert(basePath.length());

#if BS_DEBUG_LEVEL > 0
	if (mFileSystem)
	{
		bsLog::logMessage("Attempting to initialize file system multiple times, "
			"memory will leak and other problems may arise", pantheios::SEV_CRITICAL);
	}
#endif

	assert(!mFileSystem && "Attempting to initialize file system multiple times");

	mFileSystem = new bsFileSystem(basePath);
}

void bsResourceManager::initShaderManager(bsDx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);

#if BS_DEBUG_LEVEL > 0
	if (mShaderManager)
	{
		bsLog::logMessage("Attempting to initialize shader manager multiple times, "
			"memory will leak and other problems may arise", pantheios::SEV_CRITICAL);
	}
#endif

	assert(!mShaderManager && "Attempting to initialize shader manager multiple times");

	mShaderManager = new bsShaderManager(dx11Renderer, this);
}

void bsResourceManager::initMeshManager(bsDx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);

#if BS_DEBUG_LEVEL > 0
	if (mMeshManager)
	{
		bsLog::logMessage("Attempting to initialize mesh manager multiple times, "
			"memory will leak and other problems may arise", pantheios::SEV_CRITICAL);
	}
#endif

	assert(!mMeshManager && "Attempting to initialize mesh manager multiple times");

	mMeshManager = new bsMeshManager(dx11Renderer, this);
}

void bsResourceManager::initTextManager(bsDx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);

#if BS_DEBUG_LEVEL > 0
	if (mTextManager)
	{
		bsLog::logMessage("Attempting to initialize text manager multiple times, "
			"memory will leak and other problems may arise", pantheios::SEV_CRITICAL);
	}
#endif

	assert(!mTextManager && "Attempting to initialize text manager multiple times");

	mTextManager = new bsTextManager(dx11Renderer);
}
