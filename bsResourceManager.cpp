#include "StdAfx.h"

#include "bsResourceManager.h"

#include "bsLog.h"
#include "bsAssert.h"
#include "bsTextureCache.h"
#include "bsDx11Renderer.h"
#include "bsMaterialCache.h"


bsResourceManager::bsResourceManager()
	: mFileSystem(nullptr)
	, mShaderManager(nullptr)
	, mMeshCache(nullptr)
	, mTextManager(nullptr)
	, mTextureCache(nullptr)
	, mMaterialCache(nullptr)
{
}

bsResourceManager::~bsResourceManager()
{
	delete mFileSystem;
	delete mShaderManager;
	delete mMeshCache;
	delete mTextManager;
	delete mTextureCache;
	delete mMaterialCache;
}

void bsResourceManager::initAll(const std::string& fileSystemBasePath,
	bsDx11Renderer* dx11Renderer, bsFileIoManager& fileIoManager)
{
	initFileSystem(fileSystemBasePath);
	initShaderManager(dx11Renderer);
	initMeshCache(dx11Renderer, fileIoManager);
	initTextManager(dx11Renderer);
	initTextureCache(*dx11Renderer->getDevice(), fileIoManager);
	initMaterialCache();
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

	BS_ASSERT2(mFileSystem != nullptr, "File system must be initialized before shader manager");

	std::string shaderCachePath(mFileSystem->getBasePath());
	shaderCachePath.append("shaders\\precompiled");

	mShaderManager = new bsShaderManager(*dx11Renderer, *mFileSystem, shaderCachePath);
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

void bsResourceManager::initTextureCache(ID3D11Device& device, bsFileIoManager& fileIoManager)
{
	BS_ASSERT2(!mTextureCache, "Attempting to initialize texture cache multiple times, "
		"memory will leak and other problems may arise");

	BS_ASSERT2(mFileSystem != nullptr, "File system must be initialized before texture cache");

	mTextureCache = new bsTextureCache(device, *mFileSystem, fileIoManager);
}

void bsResourceManager::initMaterialCache()
{
	BS_ASSERT2(!mMaterialCache, "Attempting to initialize material cache multiple times, "
		"memory will leak and other problems may arise");

	mMaterialCache = new bsMaterialCache();
}
