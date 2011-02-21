#include "ShaderManager.h"

#include <cassert>
#include <sstream>


ShaderManager::ShaderManager(Dx11Renderer* dx11Renderer)
	: mDx11Renderer(dx11Renderer)
	, mNumCreatedShaders(0)
{
	
}

ShaderManager::~ShaderManager()
{
	/*
	for (auto itr = mPixelShaders.begin(); itr != mPixelShaders.end(); ++itr)
	{
		itr->second.get()->mPixelShader->Release();
	}
	
	for (auto itr = mVertexShaders.begin(); itr != mVertexShaders.end(); ++itr)
	{
		itr->second->mVertexShader->Release();
		itr->second->mInputLayout->Release();
	}
	
	mPixelShaders.clear();
	mVertexShaders.clear();
	*/
}

const std::shared_ptr<VertexShader> ShaderManager::getVertexShader(const std::string& name)
{
	assert(name.length());

	//See if the shader already exists
	auto val = mVertexShaders.find(name);
	if (val != mVertexShaders.end())
		return val->second;

	//Not found, create and return the shader
	return createVertexShader(name);
}

const std::shared_ptr<PixelShader> ShaderManager::getPixelShader(const std::string& name)
{
	assert(name.length());

	//See if the shader already exists
	auto val = mPixelShaders.find(name);
	if (val != mPixelShaders.end())
		return val->second;

	//Not found, create and return the shader
	return createPixelShader(name);
}

std::shared_ptr<VertexShader> ShaderManager::createVertexShader(const std::string& fileName)
{
	//TODO: Return a default fallback shader if this fails.

	//Compile shader
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

	//Create shader
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
	
	//Create input layout
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

	auto vs = std::make_pair(fileName, std::make_shared<VertexShader>(vertexShader,
		vertexLayout, getNumCreatedShaders()));

#ifdef _DEBUG
	//Set debug data in the D3D objects.

	std::stringstream bufferName("VertexShader_");
	bufferName << fileName;
	const char* vsNameCstr = bufferName.str().c_str();
	vs.second->mVertexShader->SetPrivateData(WKPDID_D3DDebugObjectName,
		sizeof(vsNameCstr - 1), vsNameCstr);

	bufferName.str("");
	bufferName << "InputLayout_" << fileName;
	const char* layoutNameCstr = bufferName.str().c_str();
	vs.second->mInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
		sizeof(layoutNameCstr - 1), layoutNameCstr);
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
	if (!SUCCEEDED(mDx11Renderer->compileShader(fileName.c_str(), "PS", "ps_4_0", &psBlob)))
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

	auto ps = std::make_pair(fileName, std::make_shared<PixelShader>(pixelShader,
		getNumCreatedShaders()));

#ifdef _DEBUG
	//Set debug data in the D3D object.

	std::stringstream bufferName("PixelShader_");
	bufferName << fileName;
	const char* psNameCstr = bufferName.str().c_str();
	ps.second->mPixelShader->SetPrivateData(WKPDID_D3DDebugObjectName,
		sizeof(psNameCstr - 1), psNameCstr);
#endif // _DEBUG

	mPixelShaders.insert(ps);

	std::string logMessage("Created pixel shader \'");
	logMessage.append(fileName);
	logMessage.append("\'");
	Log::logMessage(logMessage.c_str());

	return ps.second;
}
