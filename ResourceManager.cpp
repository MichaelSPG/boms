#include "ResourceManager.h"

ResourceManager::ResourceManager()
	: mShaderManager(nullptr)
	, mMeshManager(nullptr)
{

}

ResourceManager::~ResourceManager()
{
	if (mShaderManager)
	{
		delete mShaderManager;
	}
	if (mMeshManager)
	{
		delete mMeshManager;
	}
}

void ResourceManager::initShaderManager(Dx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);

	if (!mShaderManager)
	{
		mShaderManager = new ShaderManager(dx11Renderer);
	}
}

void ResourceManager::initMeshManager(Dx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);

	if (!mMeshManager)
	{
		mMeshManager = new MeshManager(dx11Renderer);
	}
}

