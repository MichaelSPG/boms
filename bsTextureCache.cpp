#include "StdAfx.h"

#include "bsTextureCache.h"
#include "bsFileIoManager.h"
#include "bsAssert.h"
#include "bsTexture2D.h"
#include "bsLog.h"
#include "bsFixedSizeString.h"
#include "bsFileSystem.h"


//Name for the default pink/yellow checker texture.
const char* bsTextureCacheDefaultTextureName = "__internal_default_pink_yellow_checker";



/*	Function object passed to file loader when loading textures asynchronously.
	Converts the loaded data into a texture.
*/
class bsTextureFileLoadFinishedCallback
{
public:
	bsTextureFileLoadFinishedCallback(const std::shared_ptr<bsTexture2D>& texture,
		const std::string& textureName, ID3D11Device& device)
		: mTexture(texture)
		, mTextureName(textureName)
		, mDevice(device)
	{
	}

	void operator()(const bsFileLoader& fileLoader)
	{
		BS_ASSERT(fileLoader.getCurrentLoadState() == bsFileLoader::SUCCEEDED);

		const unsigned long dataSize = fileLoader.getLoadedDataSize();
		const char* loadedData = fileLoader.getLoadedData();

		ID3D11ShaderResourceView* shaderResourceView = nullptr;
		const HRESULT hres = D3DX11CreateShaderResourceViewFromMemory(&mDevice, loadedData,
			dataSize, nullptr, nullptr, &shaderResourceView, nullptr);

		BS_ASSERT2(SUCCEEDED(hres), "Shader resource view creation failed");

		mTexture->loadingCompleted(shaderResourceView, SUCCEEDED(hres));

		bsLog::logf(bsLog::SEV_INFO, "Loading of texture '%s' finished, success: %u",
			fileLoader.getFileName().c_str(),
			fileLoader.getCurrentLoadState() == bsFileLoader::SUCCEEDED);

#ifdef BS_DEBUG
		if (shaderResourceView != nullptr)
		{
			bsString256 debugString(mTextureName.c_str());
			shaderResourceView->SetPrivateData(WKPDID_D3DDebugObjectName,
				debugString.size(), debugString.c_str());
		}
#endif
	}

private:
	std::shared_ptr<bsTexture2D> mTexture;
	std::string				mTextureName;
	ID3D11Device&			mDevice;
};


bsTextureCache::bsTextureCache(ID3D11Device& device, const bsFileSystem& fileSystem,
	bsFileIoManager& fileIoManager)
	: mFileSystem(fileSystem)
	, mFileIoManager(fileIoManager)
	, mDevice(device)
	, mNumLoadedTextures(0)
{
	/*	96 bytes of 2x2 checker texture PNG.
		Upper left and lower right pixels are yellow, other two are pink.

		This is used as a default placeholder texture while loading the actual texture
		in the background. The colors can also help with identifying textures which failed
		to load (they will always have the default yellow/pink checker texture.
	*/
	const unsigned char yellowPinkCheckerPng[] =
	{
		0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 
		0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 
		0x08, 0x02, 0x00, 0x00, 0x00, 0xfd, 0xd4, 0x9a, 0x73, 0x00, 0x00, 0x00, 
		0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x0e, 0xc4, 0x00, 0x00, 0x0e, 
		0xc4, 0x01, 0x95, 0x2b, 0x0e, 0x1b, 0x00, 0x00, 0x00, 0x12, 0x49, 0x44, 
		0x41, 0x54, 0x78, 0xda, 0x63, 0xf8, 0xff, 0x89, 0xe1, 0x3f, 0x04, 0xfe, 
		0xff, 0xc4, 0x00, 0x00, 0x39, 0x1e, 0x07, 0xdf, 0xbd, 0x59, 0x87, 0x88, 
		0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82, 
	};

	//Create the default texture.
	ID3D11ShaderResourceView* shaderResourceView = nullptr;
	const HRESULT hres = D3DX11CreateShaderResourceViewFromMemory(&mDevice,
		yellowPinkCheckerPng, sizeof(yellowPinkCheckerPng), nullptr, nullptr,
		&shaderResourceView, nullptr);

	BS_ASSERT2(SUCCEEDED(hres), "Failed to create default yellow/pink checker texture");

#ifdef BS_DEBUG
	bsString32 debugString("Texture cache default texture");
	shaderResourceView->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugString.size(), debugString.c_str());
#endif

	//Insert the default texture into texture cache.
	std::shared_ptr<bsTexture2D> defaultTexture(
		std::make_shared<bsTexture2D>(shaderResourceView, mDevice, getNewTextureId(),
		D3D11_FILTER_MIN_MAG_MIP_POINT));

	mTextures.insert(std::move(StringTexturePair(bsTextureCacheDefaultTextureName, defaultTexture)));
}

bsTextureCache::~bsTextureCache()
{
#ifdef BS_DEBUG
	//Warn if there are external references to any textures.
	for (auto itr = mTextures.begin(), end = mTextures.end(); itr != end; ++itr)
	{
		if (!itr->second.unique())
		{
			bsLog::logf(bsLog::SEV_WARNING, "All references to texture '%s' have not"
				" been released when bsTextureCache is being destroyed (%u external refs)",
				itr->first.c_str(), itr->second.use_count() - 1);
		}
	}
#endif //ifdef BS_DEBUG

	mTextures.clear();
}

std::shared_ptr<bsTexture2D> bsTextureCache::getTexture(const char* fileName)
{
	const auto findResult = mTextures.find(fileName);
	if (findResult != std::end(mTextures))
	{
		//Found it in cache.
		return findResult->second;
	}

	//Not found in cache, load it asynchronously.

	const std::string fullFilePath(mFileSystem.getPathFromFilename(fileName));
	
	if (fullFilePath.empty())
	{
		bsLog::logf(bsLog::SEV_ERROR, "Failed to find path for file '%s'", fileName);

		BS_ASSERT2(false, "Failed to find texture file");

		//Failed to load the requested texture, return default texture to prevent crashing.
		return getDefaultTexture();
	}

	//Create a temporary default placeholder while loading the actual texture in the background.
	std::shared_ptr<bsTexture2D> texture(std::move(std::make_shared<bsTexture2D>(
		mTextures[bsTextureCacheDefaultTextureName]->getShaderResourceView(), mDevice,
		getNewTextureId())));

	mTextures.insert(StringTexturePair(fileName, texture));

	mFileIoManager.addAsynchronousLoadRequest(fullFilePath,
		bsTextureFileLoadFinishedCallback(texture, fileName, mDevice));

	return texture;
}

std::shared_ptr<bsTexture2D> bsTextureCache::getTextureBlocking(const char* fileName)
{
	const auto findResult = mTextures.find(fileName);
	if (findResult != std::end(mTextures))
	{
		//Found it in cache.
		return findResult->second;
	}

	//Not found in cache, load it synchronously.

	const std::string fullFilePath(mFileSystem.getPathFromFilename(fileName));

	if (fullFilePath.empty())
	{
		bsLog::logf(bsLog::SEV_ERROR, "Failed to find path for file '%s'", fileName);

		BS_ASSERT2(false, "Failed to find texture file");

		//Failed to load the requested texture, return default texture to prevent crashing.
		return getDefaultTexture();
	}

	//Create a temporary default placeholder.
	std::shared_ptr<bsTexture2D> texture(std::move(std::make_shared<bsTexture2D>(
		mTextures[bsTextureCacheDefaultTextureName]->getShaderResourceView(), mDevice,
		getNewTextureId())));

	mTextures.insert(StringTexturePair(fileName, texture));

	//Do the same as with async loading, but call loadBlocking instead.
	bsTextureFileLoadFinishedCallback finishedCallback(texture, fileName, mDevice);
	finishedCallback(mFileIoManager.loadBlocking(fullFilePath));

	return texture;
}

std::shared_ptr<bsTexture2D> bsTextureCache::getDefaultTexture() const
{
	BS_ASSERT2(mTextures.find(bsTextureCacheDefaultTextureName) != std::end(mTextures),
		"Failed to find default texture. This should never happen");

	return mTextures.find(bsTextureCacheDefaultTextureName)->second;
}
