#ifndef BS_SHADER_MANAGER_H
#define BS_SHADER_MANAGER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <d3d11.h>

#include "bsVertexShader.h"
#include "bsPixelShader.h"

class bsResourceManager;
class bsDx11Renderer;


class bsShaderManager
{
	friend class bsResourceManager;

	bsShaderManager(bsDx11Renderer* dx11Renderer, bsResourceManager* resourceManager);
	~bsShaderManager();

public:
	//Get a shader or create it if it doesn't exist
	std::shared_ptr<bsVertexShader> getVertexShader(const std::string& fileName,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc);

	//Get a shader or create it if it doesn't exist
	std::shared_ptr<bsPixelShader> getPixelShader(const std::string& fileName);

	/*	Set a vertex shader for rendering.
		Will also set the input layout.
	*/
	void setVertexShader(const std::shared_ptr<bsVertexShader>& vertexShader);

	//Set a pixel shader for rendering.
	void setPixelShader(const std::shared_ptr<bsPixelShader>& pixelShader);


private:
	//Creation
	std::shared_ptr<bsVertexShader> createVertexShader(const std::string& fileName,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc);

	//Creation
	std::shared_ptr<bsPixelShader> createPixelShader(const std::string& fileName);


	//Increments number of created shaders and returns it.
	inline unsigned int getUniqueShaderID()
	{
		return ++mNumCreatedShaders;
	}


	std::map<std::string, std::shared_ptr<bsVertexShader>>	mVertexShaders;
	std::map<std::string, std::shared_ptr<bsPixelShader>>	mPixelShaders;

	bsResourceManager*	mResourceManager;
	bsDx11Renderer*		mDx11Renderer;
	unsigned int		mNumCreatedShaders;
};

#endif // BS_SHADER_MANAGER_H
