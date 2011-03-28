#include "MeshManager.h"

#include <sstream>

#include "ResourceManager.h"
#include "Log.h"
#include "Dx11Renderer.h"
#include "VertexTypes.h"
#include "MeshSerializer.h"



MeshManager::MeshManager(Dx11Renderer* dx11Renderer, ResourceManager* resourceManager)
	: mNumCreatedMeshes(0u)
	, mDx11Renderer(dx11Renderer)
	, mResourceManager(resourceManager)
{
	assert(dx11Renderer);
}

MeshManager::~MeshManager()
{

}

std::shared_ptr<Mesh> MeshManager::getMesh(const std::string& meshName)
{
	assert(meshName.length());

	//Get the path for the file
	std::string meshPath = mResourceManager->getFileSystem()->getPath(meshName);
	if (!meshPath.length())
	{
		//Log error message if mesh doesn't exist.

		std::string message("MeshManager: '");
		message += meshName + "' does not exist in any known resource paths,"
			" it will not be created";

		Log::logMessage(message.c_str(), pantheios::SEV_ERROR);

		return nullptr;
	}

	//See if the mesh already exists
	auto val = mMeshes.find(meshPath);
	if (val != mMeshes.end())
	{
		return val->second;
	}

	//Not found, create and return the shader.
	std::shared_ptr<Mesh> mesh = loadMesh(meshPath);
	if (!mesh)
	{
		std::stringstream message;
		message << "Something went wrong while creating '" << meshName << '\'';
		Log::logMessage(message.str().c_str(), pantheios::SEV_ERROR);

		return nullptr;
	}
	mesh->mName = meshName;

	return mesh;
}

std::shared_ptr<Mesh> MeshManager::loadMesh(const std::string& meshName)
{
	MeshSerializer meshSerializer;
	SerializedMesh serializedMesh;
	bool success = meshSerializer.load(meshName, serializedMesh);
	assert(success && "MeshManager::loadMesh failed");

	if (!success)
	{
		//MeshSerializer has logged something, just return
		return nullptr;
	}

	assert(serializedMesh.indices.size() == serializedMesh.vertices.size());
	const unsigned int meshCount = serializedMesh.indices.size();
	std::vector<ID3D11Buffer*> vertexBuffers(meshCount);
	std::vector<ID3D11Buffer*> indexBuffers(meshCount);

	for (unsigned int i = 0u; i < meshCount; ++i)
	{
		if (!createBuffers(serializedMesh.vertices[i], serializedMesh.indices[i],
			vertexBuffers[i], indexBuffers[i], meshName))
		{
			std::stringstream message;
			message << "Failed to create buffers from '" << meshName << '\'';
			Log::logMessage(message.str().c_str(), pantheios::SEV_ERROR);

			return nullptr;
		}
	}

	std::stringstream message;
	message << "Loaded mesh '" << meshName << "'";
	Log::logMessage(message.str().c_str(), pantheios::SEV_NOTICE);

	std::shared_ptr<Mesh> mesh(new Mesh());
	mesh->mID = getNumCreatedMeshes();
	mesh->mSubMeshes.resize(meshCount);

	for (unsigned int i = 0u; i < meshCount; ++i)
	{
		mesh->mSubMeshes[i] = new Mesh();
		mesh->mSubMeshes[i]->mID = getNumCreatedMeshes();
		mesh->mSubMeshes[i]->mIndexBuffer = indexBuffers[i];
		mesh->mSubMeshes[i]->mVertexBuffer = vertexBuffers[i];
		mesh->mSubMeshes[i]->mIndices = serializedMesh.indices[i].size();

		XMFLOAT3 min = serializedMesh.minExtents[i];
		XMFLOAT3 max = serializedMesh.maxExtents[i];
		mesh->mSubMeshes[i]->mAabb.m_min.set(min.x, min.y, min.z, 0.0f);
		mesh->mSubMeshes[i]->mAabb.m_max.set(max.x, max.y, max.z, 0.0f);
	}

	mesh->updateAABB();
#if BS_DEBUG_LEVEL > 4
	mesh->createDrawableAabb(mDx11Renderer, mResourceManager->getShaderManager());
#endif

	mMeshes[meshName] = mesh;
	return mesh;
}

bool MeshManager::createBuffers(const std::vector<VertexNormalTex>& vertices,
	const std::vector<unsigned int>& indices, ID3D11Buffer*& vertexBuffer,
	ID3D11Buffer*& indexBuffer, const std::string& meshName)
{
	//Vertex buffer
	D3D11_BUFFER_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(bufferDescription));
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(VertexNormalTex) * vertices.size();
	bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = &vertices[0];

	if (FAILED(mDx11Renderer->getDevice()->CreateBuffer(&bufferDescription, &initData,
		&vertexBuffer)))
	{
		Log::logMessage("Failed to create vertex buffer, aborting mesh creation",
			pantheios::SEV_ERROR);
		
		return false;
	}

	//Index buffer
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(unsigned int) * indices.size();
	bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	initData.pSysMem = &indices[0];

	if (FAILED(mDx11Renderer->getDevice()->CreateBuffer(&bufferDescription, &initData,
		&indexBuffer)))
	{
		Log::logMessage("Failed to create index buffer, aborting mesh creation",
			pantheios::SEV_ERROR);
		
		return false;
	}
#ifdef _DEBUG
	std::string debugString("VertexBuffer_");
	debugString.append(meshName);
	vertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());

	debugString = "IndexBuffer_";
	debugString.append(meshName);
	indexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());
#endif
	
	return true;
}
