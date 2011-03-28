#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Dx11Renderer.h"

#include "VertexShader.h"
#include "PixelShader.h"
//class VertexShader;
//class PixelShader;

class ResourceManager;

class ShaderManager
{
	friend class ResourceManager;

	ShaderManager(Dx11Renderer* dx11Renderer, ResourceManager* resourceManager);
	~ShaderManager();

public:
	//Get a shader or create it if it doesn't exist
	const std::shared_ptr<VertexShader> getVertexShader(const std::string& fileName,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc);

	//Get a shader or create it if it doesn't exist
	const std::shared_ptr<PixelShader> getPixelShader(const std::string& fileName);
	
	//Returns number of created shaders, but does not increment the stored value.
	inline int getNumCreatedShadersNoIncr() const
	{
		return mNumCreatedShaders;
	}


	/*	Set a vertex shader for rendering.
		Will also set the input layout.
	*/
	inline void setVertexShader(const std::shared_ptr<VertexShader>& vertexShader)
	{
		setVertexShader(vertexShader.get());
	}

	/*	Set a vertex shader for rendering.
		Will also set the input layout.
	*/
	void setVertexShader(VertexShader* vertexShader);

	//Set a pixel shader for rendering.
	inline void setPixelShader(const std::shared_ptr<PixelShader>& pixelShader)
	{
		setPixelShader(pixelShader.get());
	}

	//Set a pixel shader for rendering.
	void setPixelShader(PixelShader* pixelShader);

private:
	//Creation
	std::shared_ptr<VertexShader> createVertexShader(const std::string& fileName,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc);

	//Creation
	std::shared_ptr<PixelShader> createPixelShader(const std::string& fileName);


	//Increments number of created shaders and returns it.
	inline unsigned int getNumCreatedShaders()
	{
		return ++mNumCreatedShaders;
	}


	std::map<std::string, std::shared_ptr<VertexShader>>	mVertexShaders;
	std::map<std::string, std::shared_ptr<PixelShader>>		mPixelShaders;

	ResourceManager*	mResourceManager;
	Dx11Renderer*		mDx11Renderer;
	unsigned int		mNumCreatedShaders;
};

#endif // SHADER_MANAGER_H