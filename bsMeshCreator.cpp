#include "bsMeshCreator.h"

#include "bsLog.h"
#include "bsDx11Renderer.h"
#include "bsMeshSerializer.h"
#include "bsAssert.h"
#include "bsMesh.h"
#include "bsMeshCache.h"
#include "bsDx11Renderer.h"


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
	//Keep old buffers, maybe save some reallocation.
	mSerializedMesh.clear(false);
	mSerializedMesh = bsSerializedMesh();

	const bool success = bsLoadSerializedMesh(meshName, mSerializedMesh);
	BS_ASSERT2(success, "Failed to load serialized mesh");

	if (!success)
	{
		//bsMeshSerializer has logged something, just return
		return nullptr;
	}

	BS_ASSERT2(mSerializedMesh.indices.size() == mSerializedMesh.vertices.size(),
		"Vertex/index buffer mismatch");

	const unsigned int meshCount = mSerializedMesh.indices.size();
	std::vector<ID3D11Buffer*> vertexBuffers(meshCount);
	std::vector<ID3D11Buffer*> indexBuffers(meshCount);

	//Create buffers and check for failure for each mesh
	for (unsigned int i = 0; i < meshCount; ++i)
	{
		if (!createBuffers(vertexBuffers[i], indexBuffers[i], meshName, i))
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

	std::shared_ptr<bsMesh> mesh(std::make_shared<bsMesh>(mMeshCache.getNewMeshId(),
		meshCount, nullptr, nullptr, 0));

	std::vector<bsMesh>& subMeshes = mesh->getSubMeshes();
	subMeshes.reserve(meshCount);

	//Add all the sub meshes to the mesh
	for (unsigned int i = 0; i < meshCount; ++i)
	{
		subMeshes.push_back(std::move(bsMesh(mMeshCache.getNewMeshId(), 0, vertexBuffers[i],
			indexBuffers[i], mSerializedMesh.indices[i].size())));

		const XMFLOAT3& min = mSerializedMesh.minExtents[i];
		const XMFLOAT3& max = mSerializedMesh.maxExtents[i];

		subMeshes[i].setAabb(hkVector4(min.x, min.y, min.z), hkVector4(max.x, max.y, max.z));
	}

	mesh->updateAabb();

	return mesh;
}

bool bsMeshCreator::createBuffers(ID3D11Buffer*& vertexBuffer, ID3D11Buffer*& indexBuffer,
	const std::string& meshName, unsigned int meshIndex)
{
	BS_ASSERT(meshIndex <= mSerializedMesh.vertices.size());
	//This assert avoids unused formal parameter warning in non-debug builds.
	BS_ASSERT(!meshName.empty());

	//Index/vertex buffers of this specific mesh.
	const std::vector<VertexNormalTex>& vertices(mSerializedMesh.vertices[meshIndex]);
	const std::vector<unsigned int>& indices(mSerializedMesh.indices[meshIndex]);

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
