#include "ResourceManager.h"

ResourceManager::ResourceManager()
{

}

ResourceManager::~ResourceManager()
{

}
/*
ShaderManager& ResourceManager::getShaderManager()
{
	return *mShaderManager;
}
*/
void ResourceManager::initShaderManager(Dx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);

	mShaderManager = new ShaderManager(dx11Renderer);
}

