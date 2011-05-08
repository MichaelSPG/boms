#ifndef BS_RESOURCE_MANAGER_H
#define BS_RESOURCE_MANAGER_H

#include "bsConfig.h"

#include "bsShaderManager.h"
#include "bsMeshManager.h"
#include "bsFileSystem.h"
#include "bsTextManager.h"

class bsDx11Renderer;


class bsResourceManager
{
public:
	bsResourceManager();
	~bsResourceManager();

	//Initiates all resource managers
	void initAll(const std::string& fileSystemBasePath, bsDx11Renderer* dx11Renderer);

	void initFileSystem(const std::string& basePath);

	void initShaderManager(bsDx11Renderer* dx11Renderer);

	void initMeshManager(bsDx11Renderer* dx11Renderer);

	void initTextManager(bsDx11Renderer* dx11Renderer);

	inline bsFileSystem* getFileSystem() const
	{
		return mFileSystem;
	}

	inline bsShaderManager* getShaderManager() const
	{
		return mShaderManager;
	}

	inline bsMeshManager* getMeshManager() const
	{
		return mMeshManager;
	}

	inline bsTextManager* getTextManager() const
	{
		return mTextManager;
	}
	
private:
	bsFileSystem*		mFileSystem;
	bsShaderManager*	mShaderManager;
	bsMeshManager*	mMeshManager;
	bsTextManager*	mTextManager;
};

#endif // BS_RESOURCE_MANAGER_H
