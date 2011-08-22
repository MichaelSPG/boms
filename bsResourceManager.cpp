#include "bsResourceManager.h"

#include "bsLog.h"
#include "bsAssert.h"


bsResourceManager::bsResourceManager()
	: mFileSystem(nullptr)
	, mShaderManager(nullptr)
	, mMeshCache(nullptr)
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
	if (mMeshCache)
	{
		delete mMeshCache;
	}
	if (mTextManager)
	{
		delete mTextManager;
	}
}

void bsResourceManager::initAll(const std::string& fileSystemBasePath,
	bsDx11Renderer* dx11Renderer, bsFileIoManager& fileIoManager)
{
	initFileSystem(fileSystemBasePath);
	initShaderManager(dx11Renderer);
	initMeshCache(dx11Renderer, fileIoManager);
	initTextManager(dx11Renderer);
}

void bsResourceManager::initFileSystem(const std::string& basePath)
{
	BS_ASSERT2(!mFileSystem, "Attempting to initialize file system multiple times, "
		"memory will leak and other problems may arise");

	mFileSystem = new bsFileSystem(basePath);
}

void bsResourceManager::initShaderManager(bsDx11Renderer* dx11Renderer)
{
	BS_ASSERT(dx11Renderer);

	BS_ASSERT2(!mShaderManager, "Attempting to initialize shader manager multiple times, "
		"memory will leak and other problems may arise");

	mShaderManager = new bsShaderManager(dx11Renderer, this);
}

void bsResourceManager::initMeshCache(bsDx11Renderer* dx11Renderer, bsFileIoManager& fileIoManager)
{
	BS_ASSERT(dx11Renderer);

	BS_ASSERT2(!mMeshCache, "Attempting to initialize mesh manager multiple times, "
		"memory will leak and other problems may arise");

	mMeshCache = new bsMeshCache(dx11Renderer, *getFileSystem(), fileIoManager);
}

void bsResourceManager::initTextManager(bsDx11Renderer* dx11Renderer)
{
	BS_ASSERT(dx11Renderer);

	BS_ASSERT2(!mTextManager, "Attempting to initialize text manager multiple times, "
		"memory will leak and other problems may arise");

	mTextManager = new bsTextManager(dx11Renderer);
}
