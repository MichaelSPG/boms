#include "bsMeshCreator.h"

#include "bsLog.h"
#include "bsDx11Renderer.h"
#include "bsMeshSerializer.h"
#include "bsAssert.h"
#include "bsMesh.h"
#include "bsMeshCache.h"
#include "bsDx11Renderer.h"
#include "bsMath.h"


bsMeshCreator::bsMeshCreator(bsMeshCache& meshCache, bsDx11Renderer& dx11Renderer)
	: mMeshCache(meshCache)
	, mDx11Renderer(dx11Renderer)
{
	
}

bsMeshCreator::~bsMeshCreator()
{
	
}

std::shared_ptr<bsMesh> bsMeshCreator::loadMesh(const std::string& meshName)
{
	bsSerializedMesh serializedMesh;

	const bool success = bsLoadSerializedMesh(meshName, serializedMesh);
	BS_ASSERT2(success, "Failed to load serialized mesh");

	if (!success)
	{
		//bsMeshSerializer has logged something, just return
		return nullptr;
	}

	BS_ASSERT2(serializedMesh.indices.size() == serializedMesh.vertices.size(),
		"Vertex/index buffer mismatch");

	const unsigned int meshCount = serializedMesh.indices.size();
	std::vector<ID3D11Buffer*> vertexBuffers(meshCount);
	std::vector<ID3D11Buffer*> indexBuffers(meshCount);
	std::vector<unsigned int>  indexCounts(meshCount);

	//Create buffers and check for failure for each mesh
	for (size_t i = 0; i < meshCount; ++i)
	{
		if (!createBuffers(vertexBuffers[i], indexBuffers[i], meshName, i, serializedMesh))
		{
			std::stringstream message;
			message << "Failed to create buffers from '" << meshName << '\'';
			bsLog::logMessage(message.str().c_str(), pantheios::SEV_ERROR);

			return nullptr;
		}

		indexCounts[i] = serializedMesh.indices[i].size();
	}

	std::stringstream message;
	message << "Loaded mesh '" << meshName << "'";
	bsLog::logMessage(message.str().c_str(), pantheios::SEV_NOTICE);

	const hkAabb meshAabb(bsMath::toHK(serializedMesh.minExtents),
		bsMath::toHK(serializedMesh.maxExtents));

	std::shared_ptr<bsMesh> mesh(std::make_shared<bsMesh>(mMeshCache.getNewMeshId(),
		std::move(vertexBuffers), std::move(indexBuffers), std::move(indexCounts),
		meshAabb));
	

	return mesh;
}

bool bsMeshCreator::createBuffers(ID3D11Buffer*& vertexBuffer, ID3D11Buffer*& indexBuffer,
	const std::string& meshName, unsigned int meshIndex, bsSerializedMesh& serializedMesh)
{
	BS_ASSERT(meshIndex <= serializedMesh.vertices.size());
	//This assert avoids unused formal parameter warning in non-debug builds.
	BS_ASSERT(!meshName.empty());

	//Index/vertex buffers of this specific mesh.
	const std::vector<VertexNormalTex>& vertices(serializedMesh.vertices[meshIndex]);
	const std::vector<unsigned int>& indices(serializedMesh.indices[meshIndex]);

	//Vertex buffer
	D3D11_BUFFER_DESC bufferDescription;
	memset(&bufferDescription, 0, sizeof(bufferDescription));
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(VertexNormalTex) * vertices.size();
	bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData;
	memset(&initData, 0, sizeof(initData));
	initData.pSysMem = &vertices[0];

	if (FAILED(mDx11Renderer.getDevice()->CreateBuffer(&bufferDescription, &initData,
		&vertexBuffer)))
	{
		BS_ASSERT(!"Failed to create vertex buffer, aborting mesh creation");

		return false;
	}

	//Index buffer
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(unsigned int) * indices.size();
	bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	initData.pSysMem = &indices[0];

	if (FAILED(mDx11Renderer.getDevice()->CreateBuffer(&bufferDescription, &initData,
		&indexBuffer)))
	{
		BS_ASSERT(!"Failed to create index buffer, aborting mesh creation");

		return false;
	}

#ifdef BS_DEBUG
	//Add some debug info
	std::string debugString("VB ");
	debugString.append(meshName);
	vertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());

	debugString = "IB ";
	debugString.append(meshName);
	indexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());
#endif // BS_DEBUG

	return true;
}
