#pragma once


#include "bsShaderManager.h"
#include "bsMeshCache.h"
#include "bsFileSystem.h"
#include "bsTextManager.h"

class bsDx11Renderer;
class bsFileIoManager;
class bsTextureCache;
class bsMaterialCache;


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
	void initAll(const std::string& fileSystemBasePath, bsDx11Renderer* dx11Renderer,
		bsFileIoManager& fileIoManager);

	void initFileSystem(const std::string& basePath);

	void initShaderManager(bsDx11Renderer* dx11Renderer);

	void initMeshCache(bsDx11Renderer* dx11Renderer, bsFileIoManager& fileIoManager);

	void initTextManager(bsDx11Renderer* dx11Renderer);

	void initTextureCache(ID3D11Device& device, bsFileIoManager& fileIoManager);

	void initMaterialCache();

	inline bsFileSystem* getFileSystem() const
	{
		return mFileSystem;
	}

	inline bsShaderManager* getShaderManager() const
	{
		return mShaderManager;
	}

	inline bsMeshCache* getMeshCache() const
	{
		return mMeshCache;
	}

	inline bsTextManager* getTextManager() const
	{
		return mTextManager;
	}

	inline bsTextureCache* getTextureCache() const
	{
		return mTextureCache;
	}

	inline bsMaterialCache* getMaterialCache() const
	{
		return mMaterialCache;
	}

	
private:
	bsFileSystem*		mFileSystem;
	bsShaderManager*	mShaderManager;
	bsMeshCache*		mMeshCache;
	bsTextManager*		mTextManager;
	bsTextureCache*		mTextureCache;
	bsMaterialCache*	mMaterialCache;
};
