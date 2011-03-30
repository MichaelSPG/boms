#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "bsDx11Renderer.h"

#include "bsVertexShader.h"
#include "bsPixelShader.h"
//class bsVertexShader;
//class bsPixelShader;

class bsResourceManager;

class bsShaderManager
{
	friend class bsResourceManager;

	bsShaderManager(bsDx11Renderer* dx11Renderer, bsResourceManager* resourceManager);
	~bsShaderManager();

public:
	//Get a shader or create it if it doesn't exist
	const std::shared_ptr<bsVertexShader> getVertexShader(const std::string& fileName,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc);

	//Get a shader or create it if it doesn't exist
	const std::shared_ptr<bsPixelShader> getPixelShader(const std::string& fileName);
	
	//Returns number of created shaders, but does not increment the stored value.
	inline int getNumCreatedShadersNoIncr() const
	{
		return mNumCreatedShaders;
	}


	/*	Set a vertex shader for rendering.
		Will also set the input layout.
	*/
	inline void setVertexShader(const std::shared_ptr<bsVertexShader>& vertexShader)
	{
		setVertexShader(vertexShader.get());
	}

	/*	Set a vertex shader for rendering.
		Will also set the input layout.
	*/
	void setVertexShader(bsVertexShader* vertexShader);

	//Set a pixel shader for rendering.
	inline void setPixelShader(const std::shared_ptr<bsPixelShader>& pixelShader)
	{
		setPixelShader(pixelShader.get());
	}

	//Set a pixel shader for rendering.
	void setPixelShader(bsPixelShader* pixelShader);

private:
	//Creation
	std::shared_ptr<bsVertexShader> createVertexShader(const std::string& fileName,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc);

	//Creation
	std::shared_ptr<bsPixelShader> createPixelShader(const std::string& fileName);


	//Increments number of created shaders and returns it.
	inline unsigned int getNumCreatedShaders()
	{
		return ++mNumCreatedShaders;
	}


	std::map<std::string, std::shared_ptr<bsVertexShader>>	mVertexShaders;
	std::map<std::string, std::shared_ptr<bsPixelShader>>		mPixelShaders;

	bsResourceManager*	mResourceManager;
	bsDx11Renderer*		mDx11Renderer;
	unsigned int		mNumCreatedShaders;
};

#endif // SHADER_MANAGER_H