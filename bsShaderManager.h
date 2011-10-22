#pragma once


#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

#include <d3d11.h>

#include <Common/Base/hkBase.h>

#include "bsVertexShader.h"
#include "bsPixelShader.h"

class bsResourceManager;
class bsDx11Renderer;


/*	Contains shaders used by the application, mapped with file path as key to allow for
	fast fetching of already loaded shaders.
*/
class bsShaderManager
{
	friend class bsResourceManager;

	bsShaderManager(bsDx11Renderer* dx11Renderer, bsResourceManager* resourceManager);

	~bsShaderManager();

public:
	//Get a shader, or create it if it isn't already loaded.
	std::shared_ptr<bsVertexShader> getVertexShader(const std::string& fileName,
		const D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount) const;

	//Get a shader, or create it if it isn't already loaded.
	std::shared_ptr<bsPixelShader> getPixelShader(const std::string& fileName) const;

	/*	Set a vertex shader.
		Will also set the input layout.
	*/
	void setVertexShader(const std::shared_ptr<bsVertexShader>& vertexShader);

	//Set a pixel shader.
	void setPixelShader(const std::shared_ptr<bsPixelShader>& pixelShader);


private:
	//Create/compile vertex shader
	std::shared_ptr<bsVertexShader> createVertexShader(const std::string& fileName,
		const D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount);

	//Create/compile pixel shader
	std::shared_ptr<bsPixelShader> createPixelShader(const std::string& fileName);


	//Increments number of created shaders and returns it.
	inline unsigned int getUniqueShaderID()
	{
		return ++mNumCreatedShaders;
	}


	std::unordered_map<std::string, std::shared_ptr<bsVertexShader>>	mVertexShaders;
	std::unordered_map<std::string, std::shared_ptr<bsPixelShader>>		mPixelShaders;

	bsResourceManager*	mResourceManager;
	bsDx11Renderer*		mDx11Renderer;
	unsigned int		mNumCreatedShaders;
};
