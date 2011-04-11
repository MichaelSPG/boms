#include "bsMeshManager.h"

#include <sstream>

#include "bsResourceManager.h"
#include "bsLog.h"
#include "bsDx11Renderer.h"
#include "bsMeshSerializer.h"



bsMeshManager::bsMeshManager(bsDx11Renderer* dx11Renderer, bsResourceManager* resourceManager)
	: mNumCreatedMeshes(0)
	, mDx11Renderer(dx11Renderer)
	, mResourceManager(resourceManager)
{
	assert(dx11Renderer);
}

bsMeshManager::~bsMeshManager()
{
#if BS_DEBUG_LEVEL > 1
	//Check that there are no meshes being referenced elsewhere when the mesh manager is
	//shut down.
	for (auto itr = mMeshes.begin(), end = mMeshes.end(); itr != end; ++itr)
	{
		assert(itr->second.use_count() == 1);
	}
#endif
}

std::shared_ptr<bsMesh> bsMeshManager::getMesh(const std::string& meshName)
{
	assert(meshName.length());

	//Get the path for the file
	std::string meshPath = mResourceManager->getFileSystem()->getPath(meshName);
	if (!meshPath.length())
	{
		//Log error message if mesh doesn't exist.

		std::string message("bsMeshManager: '");
		message += meshName + "' does not exist in any known resource paths,"
			" it will not be created";

		bsLog::logMessage(message.c_str(), pantheios::SEV_ERROR);

		return nullptr;
	}

	//See if the mesh already exists
	auto val = mMeshes.find(meshPath);
	if (val != mMeshes.end())
	{
		return val->second;
	}

	//Not found, create and return the shader.
	std::shared_ptr<bsMesh> mesh = loadMesh(meshPath);
	if (!mesh)
	{
		std::stringstream message;
		message << "Something went wrong while creating '" << meshName << '\'';
		bsLog::logMessage(message.str().c_str(), pantheios::SEV_ERROR);

		return nullptr;
	}

#if BS_DEBUG_LEVEL > 1
	mesh->mName = meshName;
#endif
	mesh->mFinished = true;

	return mesh;
}

std::shared_ptr<bsMesh> bsMeshManager::loadMesh(const std::string& meshName)
{
	bsMeshSerializer meshSerializer;
	bsSerializedMesh serializedMesh;
	bool success = meshSerializer.load(meshName, serializedMesh);
	assert(success && "bsMeshManager::loadMesh failed");

	if (!success)
	{
		//bsMeshSerializer has logged something, just return
		return nullptr;
	}

	assert(serializedMesh.indices.size() == serializedMesh.vertices.size());
	const unsigned int meshCount = serializedMesh.indices.size();
	std::vector<ID3D11Buffer*> vertexBuffers(meshCount);
	std::vector<ID3D11Buffer*> indexBuffers(meshCount);

	for (unsigned int i = 0; i < meshCount; ++i)
	{
		if (!createBuffers(serializedMesh.vertices[i], serializedMesh.indices[i],
			vertexBuffers[i], indexBuffers[i], meshName))
		{
			std::stringstream message;
			message << "Failed to create buffers from '" << meshName << '\'';
			bsLog::logMessage(message.str().c_str(), pantheios::SEV_ERROR);

			return nullptr;
		}
	}

	std::stringstream message;
	message << "Loaded mesh '" << meshName << "'";
	bsLog::logMessage(message.str().c_str(), pantheios::SEV_NOTICE);

	std::shared_ptr<bsMesh> mesh(new bsMesh());
	mesh->mID = getNumCreatedMeshes();
	mesh->mSubMeshes.resize(meshCount);

	for (unsigned int i = 0; i < meshCount; ++i)
	{
		mesh->mSubMeshes[i] = new bsMesh();
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

	mMeshes[meshName] = mesh;
	return mesh;
}

bool bsMeshManager::createBuffers(const std::vector<VertexNormalTex>& vertices,
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
		bsLog::logMessage("Failed to create vertex buffer, aborting mesh creation",
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
		bsLog::logMessage("Failed to create index buffer, aborting mesh creation",
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
