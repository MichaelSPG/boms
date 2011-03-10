#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "ShaderManager.h"
#include "MeshManager.h"

class Dx11Renderer;


class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void initShaderManager(Dx11Renderer* dx11Renderer);

	void initMeshManager(Dx11Renderer* dx11Renderer);

	inline ShaderManager* getShaderManager() const
	{
		return mShaderManager;
	}

	inline MeshManager* getMeshManager() const
	{
		return mMeshManager;
	}
	
private:
	ShaderManager*	mShaderManager;
	MeshManager*	mMeshManager;
};

#endif // RESOURCE_MANAGER_H