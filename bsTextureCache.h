#pragma once

#include <unordered_map>
#include <string>
#include <memory>

class bsTexture2D;
class bsFileSystem;
class bsFileIoManager;
class bsFileLoader;
class bsDx11Renderer;
class bsTextureFileLoadFinishedCallback;


class bsTextureCache
{
public:
	bsTextureCache(ID3D11Device& device, const bsFileSystem& fileSystem,
		bsFileIoManager& fileIoManager);

	~bsTextureCache();


	std::shared_ptr<bsTexture2D> getTexture(const char* fileName);

	std::shared_ptr<bsTexture2D> getTextureBlocking(const char* fileName);
	
	std::shared_ptr<bsTexture2D> getDefaultTexture() const;

private:
	//Non-copyable.
	bsTextureCache(const bsTextureCache&);
	bsTextureCache& operator=(const bsTextureCache&);


	typedef std::pair<std::string, std::shared_ptr<bsTexture2D>> StringTexturePair;

	//Mapping of file name to texture resource.
	std::unordered_map<std::string, std::shared_ptr<bsTexture2D>> mTextures;


	/*	Used for giving new textures unique IDs.
	*/
	inline unsigned int getNewTextureId()
	{
		return ++mNumLoadedTextures;
	}


	const bsFileSystem&	mFileSystem;
	bsFileIoManager&	mFileIoManager;
	ID3D11Device&		mDevice;

	unsigned int		mNumLoadedTextures;
};
