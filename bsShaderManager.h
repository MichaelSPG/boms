#pragma once


#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

#include <d3d11.h>

#include <Common/Base/hkBase.h>

#include "bsVertexShader.h"
#include "bsPixelShader.h"

class bsFileSystem;
class bsDx11Renderer;


/*	Contains shaders used by the application, mapped with file path as key to allow for
	fast fetching of already loaded shaders.
*/
class bsShaderManager
{
public:
	bsShaderManager(bsDx11Renderer& dx11Renderer, const bsFileSystem& fileSystem,
		const std::string& precompiledShaderDirectory);

	
	/*	Set a vertex shader.
		Will also set the input layout.
	*/
	void setVertexShader(const std::shared_ptr<bsVertexShader>& vertexShader);

	//Set a pixel shader.
	void setPixelShader(const std::shared_ptr<bsPixelShader>& pixelShader);


	//Get a shader, or create it if it isn't already loaded.
	std::shared_ptr<bsVertexShader> getVertexShader(const std::string& fileName,
		const D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount) const;

	//Get a shader, or create it if it isn't already loaded.
	std::shared_ptr<bsPixelShader> getPixelShader(const std::string& fileName) const;


private:
	//Create/compile vertex shader
	std::shared_ptr<bsVertexShader> createVertexShader(const std::string& fileName,
		const D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount);

	bool compileVertexShaderBlobFromFile(ID3DBlob** blobOut, const std::string& fileName,
		const D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount);

	/*	Create vertex shader from a shader blob.
		The fileName parameter is only used for debugging and logging.
	*/
	std::shared_ptr<bsVertexShader> createVertexShaderFromBlob(ID3DBlob* blob,
		const std::string& fileName, const D3D11_INPUT_ELEMENT_DESC* inputDescs,
		unsigned int inputDescCount);



	std::shared_ptr<bsPixelShader> createPixelShader(const std::string& fileName);

	bool compilePixelShaderBlobFromFile(ID3DBlob** blobOut, const std::string& fileName);

	/*	Create pixel shader from a shader blob.
		The fileName parameter is only used for debugging and logging.
	*/
	std::shared_ptr<bsPixelShader> createPixelShaderFromBlob(ID3DBlob* blob,
		const std::string& fileName);


	/*	Returns the path for the precompiled version of the shader specified by fileName.
		This function does not append a file name, use the two functions below this one.
	*/
	inline std::string getPrecompiledShaderPath(const std::string& filePath) const;

	
	std::string getPrecompiledVertexShaderPath(const std::string& filePath) const;
	std::string getPrecompiledPixelShaderPath(const std::string& filePath) const;


	bool saveCompiledShaderToFile(const char* fileName, ID3DBlob& shaderBlobToSave) const;

	/*	Loads a shader blob from file. The shaderBlobOut should be null. If the function
		succeeds, the shaderBlobOut will be modified, if the function fails it will be null.
	*/
	void loadCompiledShaderFromFile(const char* fileName, ID3DBlob*& shaderBlobOut) const;


	//Increments number of created shaders and returns it.
	inline unsigned int getUniqueShaderID()
	{
		return ++mNumCreatedShaders;
	}


	//Mappings of file names to shaders.
	std::unordered_map<std::string, std::shared_ptr<bsVertexShader>>	mVertexShaders;
	std::unordered_map<std::string, std::shared_ptr<bsPixelShader>>		mPixelShaders;

	const bsFileSystem&	mFileSystem;
	bsDx11Renderer*		mDx11Renderer;
	unsigned int		mNumCreatedShaders;
	std::string			mPrecompiledShaderDirectory;
};
