#include "bsShaderManager.h"

#include <assert.h>
#include <string>

#include <D3Dcompiler.h>

#include "bsResourceManager.h"
#include "bsLog.h"


bsShaderManager::bsShaderManager(bsDx11Renderer* dx11Renderer, bsResourceManager* resourceManager)
	: mResourceManager(resourceManager)
	, mDx11Renderer(dx11Renderer)
	, mNumCreatedShaders(0)
{
}

bsShaderManager::~bsShaderManager()
{
}

const std::shared_ptr<bsVertexShader> bsShaderManager::getVertexShader(const std::string& fileName,
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc)
{
	assert(fileName.length());
	assert(inputDesc.size() && "Zero length input description");

	//get the path for the file
	std::string filePath = mResourceManager->getFileSystem()->getPath(fileName);
	if (!filePath.length())
	{
		//Log error message if mesh doesn't exist.

		std::string message("bsShaderManager: '");
		message += fileName + "' does not exist in any known resource paths,"
			" it will not be created";

		bsLog::logMessage(message.c_str(), pantheios::SEV_ERROR);

		assert(!"Shader does not exist");

		return nullptr;
	}

	//See if the shader already exists
	auto val = mVertexShaders.find(filePath);
	if (val != mVertexShaders.end())
	{
		return val->second;
	}

	//Not found, create and return the shader
	return createVertexShader(filePath, inputDesc);
}

const std::shared_ptr<bsPixelShader> bsShaderManager::getPixelShader(const std::string& fileName)
{
	assert(fileName.length());

	//Get the path for the file
	std::string filePath = mResourceManager->getFileSystem()->getPath(fileName);
	if (!filePath.length())
	{
		//Log error message if mesh doesn't exist.

		std::string message("bsShaderManager: '");
		message += fileName + "' does not exist in any known resource paths,"
			" it will not be created";

		bsLog::logMessage(message.c_str(), pantheios::SEV_ERROR);

		//need assert

		return nullptr;
	}

	//See if the shader already exists
	auto val = mPixelShaders.find(filePath);
	if (val != mPixelShaders.end())
	{
		return val->second;
	}

	//Not found, create and return the shader
	return createPixelShader(filePath);
}

std::shared_ptr<bsVertexShader> bsShaderManager::createVertexShader(const std::string& fileName,
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

		std::string errorMessage("Failed to compile vertex shader '");
		errorMessage += fileName + '\'';
		bsLog::logMessage(errorMessage.c_str(), pantheios::SEV_ERROR);

		assert(!"Failed to compile vertex shader");

		return nullptr;
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

		std::string errorMessage("Failed to create vertex shader '");
		errorMessage + fileName + '\'';
		bsLog::logMessage(errorMessage.c_str(), pantheios::SEV_ERROR);

		assert(!"Failed to create vertex shader");

		return nullptr;
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

		std::string errorMessage("Failed to create input layout for '");
		errorMessage += fileName + '\'';
		bsLog::logMessage(errorMessage.c_str(), pantheios::SEV_ERROR);

		assert(!"Failed to create input layout");

		return nullptr;
	}

	auto vs = std::make_pair(fileName, std::make_shared<bsVertexShader>(vertexShader,
		vertexLayout, getNumCreatedShaders()));
	vs.second->mInputLayoutDescriptions = inputDesc;

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

	std::string logMessage("Created vertex shader '");
	logMessage.append(fileName);
	logMessage.append("\'");
	bsLog::logMessage(logMessage.c_str(), pantheios::SEV_NOTICE);

	return vs.second;
}

std::shared_ptr<bsPixelShader> bsShaderManager::createPixelShader(const std::string& fileName)
{
	//Compile shader
	ID3DBlob *psBlob = nullptr;
	if (FAILED(mDx11Renderer->compileShader(fileName.c_str(), "PS", "ps_4_0", &psBlob)))
	{
		if (psBlob)
		{
			psBlob->Release();
		}

		std::string message("Failed to compile pixel shader '");
		message += fileName + '\'';
		bsLog::logMessage(message.c_str());

		return nullptr;
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

		std::string message("Failed to create pixel shader '");
		message += fileName + '\'';
		bsLog::logMessage(message.c_str(), pantheios::SEV_ERROR);

		return nullptr;
	}

	auto ps = std::make_pair(fileName, std::make_shared<bsPixelShader>(pixelShader,
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
	bsLog::logMessage(logMessage.c_str(), pantheios::SEV_NOTICE);

	return ps.second;
}

void bsShaderManager::setVertexShader(bsVertexShader* vertexShader)
{
	assert(vertexShader);

	ID3D11DeviceContext* context = mDx11Renderer->getDeviceContext();

	context->IASetInputLayout(vertexShader->getInputLayout());
	context->VSSetShader(vertexShader->getVertexShader(), nullptr, 0);
}

void bsShaderManager::setPixelShader(bsPixelShader* pixelShader)
{
	assert(pixelShader);

	ID3D11DeviceContext* context = mDx11Renderer->getDeviceContext();

	context->PSSetShader(pixelShader->getPixelShader(), nullptr, 0);
}