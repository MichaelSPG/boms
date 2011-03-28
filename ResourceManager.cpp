#include "ResourceManager.h"

#include <assert.h>


ResourceManager::ResourceManager()
	: mFileSystem(nullptr)
	, mShaderManager(nullptr)
	, mMeshManager(nullptr)
	, mTextManager(nullptr)
{

}

ResourceManager::~ResourceManager()
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

void ResourceManager::initAll( const char* fileSystemBasePath, Dx11Renderer* dx11Renderer )
{
	initFileSystem(fileSystemBasePath);
	initShaderManager(dx11Renderer);
	initMeshManager(dx11Renderer);
	initTextManager(dx11Renderer);
}

void ResourceManager::initFileSystem(const char* basePath)
{
	assert(basePath);

#if BS_DEBUG_LEVEL > 0
	if (mFileSystem)
	{
		Log::logMessage("Attempting to initialize file system multiple times, "
			"memory will leak and other problems may arise", pantheios::SEV_CRITICAL);
	}
#endif

	assert(!mFileSystem && "Attempting to initialize file system multiple times");

	mFileSystem = new FileSystem(basePath);
}

void ResourceManager::initShaderManager(Dx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);

#if BS_DEBUG_LEVEL > 0
	if (mShaderManager)
	{
		Log::logMessage("Attempting to initialize shader manager multiple times, "
			"memory will leak and other problems may arise", pantheios::SEV_CRITICAL);
	}
#endif

	assert(!mShaderManager && "Attempting to initialize shader manager multiple times");

	mShaderManager = new ShaderManager(dx11Renderer, this);
}

void ResourceManager::initMeshManager(Dx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);

#if BS_DEBUG_LEVEL > 0
	if (mMeshManager)
	{
		Log::logMessage("Attempting to initialize mesh manager multiple times, "
			"memory will leak and other problems may arise", pantheios::SEV_CRITICAL);
	}
#endif

	assert(!mMeshManager && "Attempting to initialize mesh manager multiple times");

	mMeshManager = new MeshManager(dx11Renderer, this);
}

void ResourceManager::initTextManager(Dx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);

#if BS_DEBUG_LEVEL > 0
	if (mTextManager)
	{
		Log::logMessage("Attempting to initialize text manager multiple times, "
			"memory will leak and other problems may arise", pantheios::SEV_CRITICAL);
	}
#endif

	assert(!mTextManager && "Attempting to initialize text manager multiple times");

	mTextManager = new bsTextManager(dx11Renderer);
}
