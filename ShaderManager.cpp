#include "ShaderManager.h"

#include <assert.h>
#include <sstream>

#include <D3Dcompiler.h>


ShaderManager::ShaderManager(Dx11Renderer* dx11Renderer)
	: mDx11Renderer(dx11Renderer)
	, mNumCreatedShaders(0)
{
}

ShaderManager::~ShaderManager()
{
}

const std::shared_ptr<VertexShader> ShaderManager::getVertexShader(const std::string& fileName,
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc)
{
	assert(fileName.length());
	assert(inputDesc.size() && "Zero length input description");

	//See if the shader already exists
	auto val = mVertexShaders.find(fileName);
	if (val != mVertexShaders.end())
	{
		return val->second;
	}

	//Not found, create and return the shader
	return createVertexShader(fileName, inputDesc);
}

const std::shared_ptr<PixelShader> ShaderManager::getPixelShader(const std::string& fileName)
{
	assert(fileName.length());

	//See if the shader already exists
	auto val = mPixelShaders.find(fileName);
	if (val != mPixelShaders.end())
	{
		return val->second;
	}

	//Not found, create and return the shader
	return createPixelShader(fileName);
}

std::shared_ptr<VertexShader> ShaderManager::createVertexShader(const std::string& fileName,
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc)
{
	//TODO: Return a default fallback shader if this fails.

	//Compile shader
	ID3DBlob* vsBlob = nullptr;
	if (FAILED(mDx11Renderer->compileShader(fileName.c_str(), "VS", "vs_4_0", &vsBlob)))
	{
		if (vsBlob)
		{
			vsBlob->Release();
		}
		throw std::exception(std::string(std::string("Failed to compile vertex shader with name ")
			+ fileName).c_str());
	}

	//Create shader
	ID3D11VertexShader* vertexShader = nullptr;
	if (FAILED(mDx11Renderer->getDevice()->CreateVertexShader(vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(), nullptr, &vertexShader)))
	{
		if (vsBlob)
		{
			vsBlob->Release();
		}
		throw std::exception(std::string(std::string("Failed to create vertex shader with name ")
			+ fileName).c_str());
	}

	//Create input layout
	ID3D11InputLayout* vertexLayout = nullptr;
	if (FAILED(mDx11Renderer->getDevice()->CreateInputLayout(&inputDesc[0], inputDesc.size(),
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

	auto vs = std::make_pair(fileName, std::make_shared<VertexShader>(vertexShader,
		vertexLayout, getNumCreatedShaders()));

#ifdef _DEBUG
	//Set debug data in the D3D objects.
	std::string bufferName("VertexShader_");
	bufferName.append(fileName);
	vs.second->mVertexShader->SetPrivateData(WKPDID_D3DDebugObjectName,
		bufferName.size(), bufferName.c_str());

	bufferName = "InputLayout_";
	bufferName.append(fileName);
	vs.second->mInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
		bufferName.size(), bufferName.c_str());
#endif // _DEBUG

	mVertexShaders.insert(vs);

	std::string logMessage("Created vertex shader \'");
	logMessage.append(fileName);
	logMessage.append("\'");
	Log::logMessage(logMessage.c_str());

	return vs.second;
}

std::shared_ptr<PixelShader> ShaderManager::createPixelShader(const std::string& fileName)
{
	//Compile shader
	ID3DBlob *psBlob = nullptr;
	if (FAILED(mDx11Renderer->compileShader(fileName.c_str(), "PS", "ps_4_0", &psBlob)))
	{
		if (psBlob)
		{
			psBlob->Release();
		}
		throw std::exception(std::string(std::string("Failed to compile pixel shader with name ")
			+ fileName).c_str());
	}

	//Create shader
	ID3D11PixelShader* pixelShader = nullptr;
	if (FAILED(mDx11Renderer->getDevice()->CreatePixelShader(psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(), nullptr, &pixelShader)))
	{
		if (pixelShader)
		{
			pixelShader->Release();
		}

		throw std::exception(std::string(std::string("Failed to create pixel shader with name ")
			+ fileName).c_str());
	}

	auto ps = std::make_pair(fileName, std::make_shared<PixelShader>(pixelShader,
		getNumCreatedShaders()));

#ifdef _DEBUG
	//Set debug data in the D3D object.

	std::string bufferName("PixelShader_");
	bufferName.append(fileName);
	ps.second->mPixelShader->SetPrivateData(WKPDID_D3DDebugObjectName,
		bufferName.size(), bufferName.c_str());
#endif // _DEBUG

	mPixelShaders.insert(ps);

	std::string logMessage("Created pixel shader \'");
	logMessage.append(fileName);
	logMessage.append("\'");
	Log::logMessage(logMessage.c_str());

	return ps.second;
}
