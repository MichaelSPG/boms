#include "ResourceManager.h"

ResourceManager::ResourceManager()
	: mShaderManager(nullptr)
{

}

ResourceManager::~ResourceManager()
{
	if (mShaderManager)
	{
		delete mShaderManager;
	}
}

void ResourceManager::initShaderManager(Dx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);

	mShaderManager = new ShaderManager(dx11Renderer);
}

