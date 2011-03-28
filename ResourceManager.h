#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "bsConfig.h"

#include "ShaderManager.h"
#include "MeshManager.h"
#include "FileSystem.h"
#include "bsTextManager.h"

class Dx11Renderer;


class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	//Initiates all resource managers
	void initAll(const char* fileSystemBasePath, Dx11Renderer* dx11Renderer);

	void initFileSystem(const char* basePath);

	void initShaderManager(Dx11Renderer* dx11Renderer);

	void initMeshManager(Dx11Renderer* dx11Renderer);

	void initTextManager(Dx11Renderer* dx11Renderer);

	inline FileSystem* getFileSystem() const
	{
		return mFileSystem;
	}

	inline ShaderManager* getShaderManager() const
	{
		return mShaderManager;
	}

	inline MeshManager* getMeshManager() const
	{
		return mMeshManager;
	}

	inline bsTextManager* getTextManager() const
	{
		return mTextManager;
	}
	
private:
	FileSystem*		mFileSystem;
	ShaderManager*	mShaderManager;
	MeshManager*	mMeshManager;
	bsTextManager*	mTextManager;
};

#endif // RESOURCE_MANAGER_H