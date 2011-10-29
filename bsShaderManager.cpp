#include "StdAfx.h"

#include "bsShaderManager.h"

#include <string>
#include <time.h>
#include <time.inl>

#include <D3Dcompiler.h>

#include "bsFileSystem.h"
#include "bsLog.h"
#include "bsAssert.h"
#include "bsDx11Renderer.h"
#include "bsFileUtil.h"
#include "bsTimer.h"


bsShaderManager::bsShaderManager(bsDx11Renderer& dx11Renderer, const bsFileSystem& fileSystem,
	const std::string& precompiledShaderDirectory)
	: mFileSystem(fileSystem)
	, mDx11Renderer(&dx11Renderer)
	, mNumCreatedShaders(0)
	, mPrecompiledShaderDirectory(precompiledShaderDirectory)
{
	//Create the directory for precompiled shaders.
	//This call may fail if the directory is a file, or if the folder already exists.
	CreateDirectoryA(mPrecompiledShaderDirectory.c_str(), nullptr);

	//Verify that the directory exist. If it is a file, the above call probably
	//failed to create a directory with the same name.
	if (!bsFileUtil::directoryExists(mPrecompiledShaderDirectory.c_str()))
	{
		bsLog::logMessage("Unable to create precompiled shader directory. Please ensure"
			" that the directory name is not used by a file", bsLog::SEV_CRITICAL);

		BS_ASSERT2(false, "Unable to create precompiled shader directory. Please ensure"
			" that the directory name is not used by a file");
	}
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

std::shared_ptr<bsVertexShader> bsShaderManager::getVertexShader(const std::string& fileName,
	const D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount) const
{
	BS_ASSERT(fileName.length());
	BS_ASSERT2(inputDescCount, "Zero length input description");

	//Get the path for the file
	const std::string filePath = mFileSystem.getPathFromFilename(fileName);
	if (!filePath.length())
	{
		//The path for the given mesh name was not found

		std::string message("bsShaderManager: '");
		message += fileName + "' does not exist in any known resource paths,"
			" it will not be created";

		BS_ASSERT2(false, message.c_str());

		//This is useful info even if asserts are disabled, so log it again just in case.
		bsLog::logMessage(message.c_str(), bsLog::SEV_ERROR);

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
	return const_cast<bsShaderManager*>(this)->createVertexShader(filePath, inputDescs, inputDescCount);
}

std::shared_ptr<bsVertexShader> bsShaderManager::createVertexShader(const std::string& fileName,
	const D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount)
{
	//Load shader blob and create the shader.


	ID3DBlob* blob = nullptr;
	bool usingPrecompiledShader = false;

	//See if a precompiled version of the shader exists.
	const std::string precompiledPath(std::move(getPrecompiledVertexShaderPath(fileName)));

	if (bsFileUtil::fileExists(fileName.c_str()) && bsFileUtil::fileExists(precompiledPath.c_str()))
	{
		const time_t lastModifiedPrecompiled = bsFileUtil::lastModifiedTime(precompiledPath.c_str());
		const time_t lastModifiedUncompiled = bsFileUtil::lastModifiedTime(fileName.c_str());

		//Check if precompiled version is newer than uncompiled version.
		//If it's not newer, the precompiled version is outdated and needs to be compiled
		//again.
		if (difftime(lastModifiedPrecompiled, lastModifiedUncompiled) > 0.0f)
		{
			//Precompiled version was made after the uncompiled shader, load it.
			loadCompiledShaderFromFile(precompiledPath.c_str(), blob);

			usingPrecompiledShader = blob != nullptr;
		}
	}
	if (blob == nullptr)
	{
		//Precompiled version not found or was outdated, recompile it.

		if (!compileVertexShaderBlobFromFile(&blob, fileName, inputDescs, inputDescCount))
		{
			//Compilation failed.
			return nullptr;
		}

		saveCompiledShaderToFile(precompiledPath.c_str(), *blob);
	}


	//Create the shader from the loaded blob.
	return createVertexShaderFromBlob(blob, usingPrecompiledShader ? precompiledPath : fileName,
		inputDescs, inputDescCount);
}

std::shared_ptr<bsVertexShader> bsShaderManager::createVertexShaderFromBlob(ID3DBlob* blob,
	const std::string& fileName, const D3D11_INPUT_ELEMENT_DESC* inputDescs,
	unsigned int inputDescCount)
{
	//Create shader
	ID3D11VertexShader* vertexShader = nullptr;
	if (FAILED(mDx11Renderer->getDevice()->CreateVertexShader(blob->GetBufferPointer(),
		blob->GetBufferSize(), nullptr, &vertexShader)))
	{
		if (blob)
		{
			blob->Release();
		}

		std::string errorMessage("Failed to create vertex shader '");
		errorMessage + fileName + '\'';
		bsLog::logMessage(errorMessage.c_str(), bsLog::SEV_ERROR);

		BS_ASSERT2(false, errorMessage.c_str());

		return nullptr;
	}

	//Create input layout
	ID3D11InputLayout* vertexLayout = nullptr;
	if (FAILED(mDx11Renderer->getDevice()->CreateInputLayout(inputDescs, inputDescCount,
		blob->GetBufferPointer(), blob->GetBufferSize(), &vertexLayout)))
	{
		if (blob)
		{
			blob->Release();
		}

		std::string errorMessage("Failed to create input layout for '");
		errorMessage += fileName + '\'';

		BS_ASSERT2(false, errorMessage.c_str());

		return nullptr;
	}

	auto vs = std::make_pair(fileName, std::make_shared<bsVertexShader>(vertexShader,
		vertexLayout, getUniqueShaderID()));

	vs.second->mInputLayoutDescriptions.insert(std::begin(vs.second->mInputLayoutDescriptions),
		inputDescs, inputDescs + inputDescCount);
	
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
	bsLog::logMessage(logMessage.c_str(), bsLog::SEV_INFORMATIONAL);

	return vs.second;
}

bool bsShaderManager::compileVertexShaderBlobFromFile(ID3DBlob** blobOut,
	const std::string& fileName, const D3D11_INPUT_ELEMENT_DESC* inputDescs,
	unsigned int inputDescCount)
{
	//TODO: Return a default fallback shader if this fails.

	//Compile shader
	if (FAILED(mDx11Renderer->compileShader(fileName.c_str(), "VS", "vs_4_0", blobOut)))
	{
		if (blobOut)
		{
			(*blobOut)->Release();
			*blobOut = nullptr;
		}

		std::string errorMessage("Failed to compile vertex shader '");
		errorMessage += fileName + '\'';

		BS_ASSERT2(false, errorMessage.c_str());

		return false;
	}
	return true;
}




//////////////////////////////////////////////////////////////////////////
// Pixel shader
//////////////////////////////////////////////////////////////////////////


std::shared_ptr<bsPixelShader> bsShaderManager::getPixelShader(const std::string& fileName) const
{
	BS_ASSERT2(fileName.length(), "Zero length file name. Use \".\" for current path");

	//Get the path for the file
	const std::string filePath = mFileSystem.getPathFromFilename(fileName);

	if (!filePath.length())
	{
		std::string message("'");
		message += fileName + "' does not exist in any known resource paths,"
			" it will not be created";

		BS_ASSERT2(false, message.c_str());

		//This is useful info even if asserts are disabled, so log it again just in case.
		bsLog::logMessage(message.c_str(), bsLog::SEV_ERROR);

		return nullptr;
	}

	//See if the shader already exists
	auto val = mPixelShaders.find(filePath);
	if (val != mPixelShaders.end())
	{
		return val->second;
	}

	//Create the shader from the loaded blob.
	return const_cast<bsShaderManager*>(this)->createPixelShader(filePath);
}

std::shared_ptr<bsPixelShader> bsShaderManager::createPixelShader(const std::string& fileName)
{
	//Load shader blob and create the shader.


	ID3DBlob* blob = nullptr;
	bool usingPrecompiledShader = false;

	//See if a precompiled version of the shader exists.
	const std::string precompiledPath(std::move(getPrecompiledPixelShaderPath(fileName)));

	if (bsFileUtil::fileExists(fileName.c_str()) && bsFileUtil::fileExists(precompiledPath.c_str()))
	{
		const time_t lastModifiedPrecompiled = bsFileUtil::lastModifiedTime(precompiledPath.c_str());
		const time_t lastModifiedUncompiled = bsFileUtil::lastModifiedTime(fileName.c_str());

		//Check if precompiled version is newer than uncompiled version.
		//If it's not newer, the precompiled version is outdated and needs to be compiled
		//again.
		if (difftime(lastModifiedPrecompiled, lastModifiedUncompiled) > 0.0f)
		{
			//Precompiled version was made after the uncompiled shader, load it.
			loadCompiledShaderFromFile(precompiledPath.c_str(), blob);

			usingPrecompiledShader = blob != nullptr;
		}
	}
	if (blob == nullptr)
	{
		//Precompiled version not found or was outdated, recompile it.

		if (!compilePixelShaderBlobFromFile(&blob, fileName))
		{
			//Compilation failed.
			return nullptr;
		}

		saveCompiledShaderToFile(precompiledPath.c_str(), *blob);
	}

	//Create the shader from the loaded blob.
	return createPixelShaderFromBlob(blob, usingPrecompiledShader ? precompiledPath : fileName);
}

std::shared_ptr<bsPixelShader> bsShaderManager::createPixelShaderFromBlob(ID3DBlob* blob,
	const std::string& fileName)
{
	//Create shader
	ID3D11PixelShader* pixelShader = nullptr;
	if (FAILED(mDx11Renderer->getDevice()->CreatePixelShader(blob->GetBufferPointer(),
		blob->GetBufferSize(), nullptr, &pixelShader)))
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
	bsLog::logMessage(logMessage.c_str(), bsLog::SEV_INFORMATIONAL);

	return ps.second;
}

bool bsShaderManager::compilePixelShaderBlobFromFile(ID3DBlob** blobOut,
	const std::string& fileName)
{
	if (FAILED(mDx11Renderer->compileShader(fileName.c_str(),"PS", "ps_4_0", blobOut)))
	{
		if (blobOut)
		{
			(*blobOut)->Release();
			*blobOut = nullptr;
		}

		std::string message("Failed to compile pixel shader '");
		message += fileName + '\'';

		BS_ASSERT2(false, message.c_str());

		return false;
	}
	return true;
}

bool bsShaderManager::saveCompiledShaderToFile(const char* fileName, ID3DBlob& shaderBlobToSave) const
{
	FILE* file = fopen(fileName, "wb");
	if (file != nullptr)
	{
		const unsigned int shaderBlobSize = shaderBlobToSave.GetBufferSize();
		const size_t written1 = fwrite(&shaderBlobSize, sizeof(unsigned int), 1, file);
		const size_t written2 = fwrite(shaderBlobToSave.GetBufferPointer(), shaderBlobSize, 1, file);

		fclose(file);

		return written1 == 1 && written2 == 1;
	}
	return false;
}

void bsShaderManager::loadCompiledShaderFromFile(const char* fileName, ID3DBlob*& shaderBlobOut) const
{
	FILE* file = fopen(fileName, "rb");
	if (file)
	{
		//Load first 4 bytes (size of shader blob), then the shader.

		unsigned int shaderBlobSize;
		const size_t read1 = fread(&shaderBlobSize, sizeof(unsigned int), 1, file);
		
		char* buffer = static_cast<char*>(malloc(shaderBlobSize));
		const size_t read2 = fread(buffer, shaderBlobSize, 1, file);
		
		if (read1 && read2)
		{
			//Reading succeeded, copy loaded buffer into blob.
			D3DCreateBlob(shaderBlobSize, &shaderBlobOut);
			memcpy(shaderBlobOut->GetBufferPointer(), buffer, shaderBlobSize);
		}
		
		free(buffer);
	}
}

inline std::string bsShaderManager::getPrecompiledShaderPath(const std::string& filePath) const
{
	std::string compiledShaderFileName(mPrecompiledShaderDirectory);
	if (compiledShaderFileName.back() != '\\')
	{
		compiledShaderFileName.push_back('\\');
	}
	
	//Append file name from parameter to base precompiled directory.
	compiledShaderFileName.append(filePath.substr(filePath.find_last_of('\\') + 1));
	//Erase file extension.
	compiledShaderFileName.erase(compiledShaderFileName.find_last_of('.') + 1);
#ifdef BS_DEBUG
	//Add "dbg_" to file extensions when compiling in debug to avoid name conflicts.
	compiledShaderFileName.append("dbg_");
#endif

	return std::move(compiledShaderFileName);
}

std::string bsShaderManager::getPrecompiledVertexShaderPath(const std::string& filePath) const
{
	std::string compiledShaderFileName(std::move(getPrecompiledShaderPath(filePath)));
	//Append file extension (pcvs = precompiled vertex shader).
	compiledShaderFileName.append("pcvs");

	return compiledShaderFileName;
}

std::string bsShaderManager::getPrecompiledPixelShaderPath(const std::string& filePath) const
{
	std::string compiledShaderFileName(std::move(getPrecompiledShaderPath(filePath)));
	//Append file extension (pcps = precompiled pixel shader).
	compiledShaderFileName.append("pcps");

	return compiledShaderFileName;
}
