#ifndef BS_RESOURCE_MANAGER_H
#define BS_RESOURCE_MANAGER_H

#include "bsConfig.h"

#include "bsShaderManager.h"
#include "bsMeshManager.h"
#include "bsFileSystem.h"
#include "bsTextManager.h"

class bsDx11Renderer;


/*	Contains all the resource managers used by the engine.
	You don't need to initiate every resource manager, just the ones you are going to use.
*/

//TODO: Add memory tracking/freeing functionality to this and other resource managers
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
	bsMeshManager*		mMeshManager;
	bsTextManager*		mTextManager;
};

#endif // BS_RESOURCE_MANAGER_H
