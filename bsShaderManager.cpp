#include "StdAfx.h"

#include "bsShaderManager.h"

#include <string>

#include <D3Dcompiler.h>

#include "bsResourceManager.h"
#include "bsLog.h"
#include "bsAssert.h"
#include "bsDx11Renderer.h"


bsShaderManager::bsShaderManager(bsDx11Renderer* dx11Renderer, bsResourceManager* resourceManager)
	: mResourceManager(resourceManager)
	, mDx11Renderer(dx11Renderer)
	, mNumCreatedShaders(0)
{
}

bsShaderManager::~bsShaderManager()
{
}

std::shared_ptr<bsVertexShader> bsShaderManager::getVertexShader(const std::string& fileName,
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc) const
{
	BS_ASSERT(fileName.length());
	BS_ASSERT2(inputDesc.size(), "Zero length input description");

	//Get the path for the file
	std::string filePath = mResourceManager->getFileSystem()->getPathFromFilename(fileName);
	if (!filePath.length())
	{
		//The path for the given mesh name was not found

		std::string message("bsShaderManager: '");
		message += fileName + "' does not exist in any known resource paths,"
			" it will not be created";

		BS_ASSERT2(false, message.c_str());

		//This is useful info even if asserts are disabled, so log it again just in case
		bsLog::logMessage(message.c_str(), pantheios::SEV_ERROR);

		return nullptr;
	}

	//See if the shader already exists
	auto val = mVertexShaders.find(filePath);
	if (val != mVertexShaders.end())
	{
		return val->second;
	}

	//Not found, create and return the shader
	//createVertexShader is not const, so must cast const away
	return const_cast<bsShaderManager*>(this)->createVertexShader(filePath, inputDesc);
}

std::shared_ptr<bsPixelShader> bsShaderManager::getPixelShader(const std::string& fileName) const
{
	BS_ASSERT2(fileName.length(), "Zero length file name. Use \".\" for current path");

	//Get the path for the file
	std::string filePath = mResourceManager->getFileSystem()->getPathFromFilename(fileName);

	if (!filePath.length())
	{
		std::string message("'");
		message += fileName + "' does not exist in any known resource paths,"
			" it will not be created";

		BS_ASSERT2(false, message.c_str());

		//This is useful info even if asserts are disabled, so log it again just in case
		bsLog::logMessage(message.c_str(), pantheios::SEV_ERROR);

		return nullptr;
	}

	//See if the shader already exists
	auto val = mPixelShaders.find(filePath);
	if (val != mPixelShaders.end())
	{
		return val->second;
	}

	//Not found, create and return the shader
	return const_cast<bsShaderManager*>(this)->createPixelShader(filePath);
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

		BS_ASSERT2(false, errorMessage.c_str());

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

		BS_ASSERT2(false, errorMessage.c_str());

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

		BS_ASSERT2(false, errorMessage.c_str());

		return nullptr;
	}

	auto vs = std::make_pair(fileName, std::make_shared<bsVertexShader>(vertexShader,
		vertexLayout, getUniqueShaderID()));
	vs.second->mInputLayoutDescriptions = inputDesc;

#ifdef BS_DEBUG
	//Set debug data in the D3D objects.
	std::string bufferName("VS ");
	bufferName.append(fileName);
	vs.second->mVertexShader->SetPrivateData(WKPDID_D3DDebugObjectName,
		bufferName.size(), bufferName.c_str());

	bufferName = "IL ";
	bufferName.append(fileName);
	vs.second->mInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
		bufferName.size(), bufferName.c_str());
#endif // BS_DEBUG

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

		BS_ASSERT2(false, message.c_str());

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

		BS_ASSERT2(false, message.c_str());

		return nullptr;
	}

	auto ps = std::make_pair(fileName, std::make_shared<bsPixelShader>(pixelShader,
		getUniqueShaderID()));

#ifdef BS_DEBUG
	//Set debug data in the D3D object.

	std::string bufferName("PS ");
	bufferName.append(fileName);
	ps.second->mPixelShader->SetPrivateData(WKPDID_D3DDebugObjectName,
		bufferName.size(), bufferName.c_str());
#endif // BS_DEBUG

	mPixelShaders.insert(ps);

	std::string logMessage("Created pixel shader \'");
	logMessage.append(fileName);
	logMessage.append("\'");
	bsLog::logMessage(logMessage.c_str(), pantheios::SEV_NOTICE);

	return ps.second;
}

void bsShaderManager::setVertexShader(const std::shared_ptr<bsVertexShader>& vertexShader)
{
	BS_ASSERT(vertexShader);

	ID3D11DeviceContext* context = mDx11Renderer->getDeviceContext();

	context->IASetInputLayout(vertexShader->mInputLayout);
	context->VSSetShader(vertexShader->mVertexShader, nullptr, 0);
}

void bsShaderManager::setPixelShader(const std::shared_ptr<bsPixelShader>& pixelShader)
{
	BS_ASSERT(pixelShader);

	ID3D11DeviceContext* context = mDx11Renderer->getDeviceContext();

	context->PSSetShader(pixelShader->mPixelShader, nullptr, 0);
}
