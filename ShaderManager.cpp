#include "ShaderManager.h"
#include <cassert>

ShaderManager::ShaderManager(Dx11Renderer* dx11Renderer)
	: mDx11Renderer(dx11Renderer)
{
	
}

ShaderManager::~ShaderManager()
{

}

const std::shared_ptr<VertexShader>& ShaderManager::getVertexShader(const std::string& name)
{
	assert(name.length());

	//See if the shader already exists
	auto val = mVertexShaders.find(name);
	if (val != mVertexShaders.end())
		return val->second;

	//Not found, create and return the shader
	return createVertexShader(name);
}

const std::shared_ptr<PixelShader>& ShaderManager::getPixelShader(const std::string& name)
{
	assert(name.length());

	//See if the shader already exists
	auto val = mPixelShaders.find(name);
	if (val != mPixelShaders.end())
		return val->second;

	//Not found, create and return the shader
	return createPixelShader(name);
}

std::shared_ptr<VertexShader>& ShaderManager::createVertexShader(const std::string& fileName)
{
	//TODO: Return a default fallback shader if this fails.

	ID3DBlob* vsBlob = nullptr;
	
	if (!SUCCEEDED(mDx11Renderer->compileShader(fileName.c_str(), "VS", "vs_4_0", &vsBlob)))
	{
		if (vsBlob)
		{
			vsBlob->Release();
		}
		throw std::exception(std::string(std::string("Failed to compile vertex shader with name ")
			+ fileName).c_str());
	}

	ID3D11VertexShader* vertexShader = nullptr;

	if (!SUCCEEDED(mDx11Renderer->getDevice()->CreateVertexShader(vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(), nullptr, &vertexShader)))
	{
		if (vsBlob)
		{
			vsBlob->Release();
		}
		throw std::exception(std::string(std::string("Failed to create vertex shader with name ")
			+ fileName).c_str());
	}

	D3D11_INPUT_ELEMENT_DESC layout[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	unsigned int numElements(ARRAYSIZE(layout));
	
	ID3D11InputLayout* vertexLayout = nullptr;

	if (!SUCCEEDED(mDx11Renderer->getDevice()->CreateInputLayout(layout, numElements,
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &vertexLayout)))
	{
		if (vsBlob)
		{
			vsBlob->Release();
		}
		std::stringstream sstream;
		sstream << "Failed to create input layout for file " << fileName.c_str();
		throw std::exception(sstream.str().c_str());
	}

	auto vs = std::make_pair(fileName, std::make_shared<VertexShader>(vertexShader, vertexLayout));
	//auto vs = std::make_pair(fileName, std::shared_ptr<VertexShader>(vertexShader, nullptr));
	mVertexShaders.insert(vs);

	return mVertexShaders.find(fileName)->second;
}

std::shared_ptr<PixelShader>& ShaderManager::createPixelShader(const std::string& fileName)
{
	ID3DBlob *psBlob = nullptr;

	if (!SUCCEEDED(mDx11Renderer->compileShader(fileName.c_str(), "PS", "ps_4_0", &psBlob)))
	{
		if (psBlob)
		{
			psBlob->Release();
		}
		throw std::exception(std::string(std::string("Failed to compile pixel shader with name ")
			+ fileName).c_str());
	}

	ID3D11PixelShader* pixelShader = nullptr;

	if (!SUCCEEDED(mDx11Renderer->getDevice()->CreatePixelShader(psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(), nullptr, &pixelShader)))
	{
		if (pixelShader)
		{
			pixelShader->Release();
		}

		throw std::exception(std::string(std::string("Failed to create pixel shader with name ")
			+ fileName).c_str());
	}


	auto vs = std::make_pair(fileName, std::make_shared<PixelShader>(pixelShader));
	mPixelShaders.insert(vs);

	return mPixelShaders.find(fileName)->second;
}
