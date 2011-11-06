#include "StdAfx.h"

#include "bsMeshCreator.h"
#include "bsLog.h"
#include "bsDx11Renderer.h"
#include "bsMeshSerializer.h"
#include "bsAssert.h"
#include "bsMesh.h"
#include "bsMeshCache.h"
#include "bsDx11Renderer.h"
#include "bsMath.h"
#include "bsFileSystem.h"
#include "bsFileIoManager.h"


/*	Function object passed to file loader when loading meshes asynchronously.
	Converts the loaded data into a mesh.
*/
class bsMeshCreatorFileLoadFinished
{
public:
	bsMeshCreatorFileLoadFinished(const std::shared_ptr<bsMesh>& mesh,
		const std::string& meshName, const bsMeshCreator& meshCreator)
		: mMesh(mesh)
		, mMeshName(meshName)
		, mMeshCreator(meshCreator)
	{
	}

	void operator()(const bsFileLoader& fileLoader)
	{
		BS_ASSERT(fileLoader.getCurrentLoadState() == bsFileLoader::SUCCEEDED);

		const unsigned long dataSize = fileLoader.getLoadedDataSize();
		const char* loadedData = fileLoader.getLoadedData();

		bsSerializedMesh serializedMesh;
		bsLoadSerializedMeshFromMemory(loadedData, dataSize, serializedMesh);
		
		*mMesh = std::move(*mMeshCreator.constructMeshFromSerializedMesh(serializedMesh, mMeshName));
	}

private:
	std::shared_ptr<bsMesh>	mMesh;
	std::string				mMeshName;
	const bsMeshCreator&	mMeshCreator;
};


bsMeshCreator::bsMeshCreator(bsMeshCache& meshCache, const bsDx11Renderer& dx11Renderer,
	const bsFileSystem& fileSystem, bsFileIoManager& fileManager)
	: mMeshCache(meshCache)
	, mD3dDevice(dx11Renderer.getDevice())
	, mFileSystem(fileSystem)
	, mFileManager(fileManager)
{
	mD3dDevice->AddRef();
}

bsMeshCreator::~bsMeshCreator()
{
	mD3dDevice->Release();
}

std::shared_ptr<bsMesh> bsMeshCreator::loadMeshAsync(const std::string& meshName)
{
	std::shared_ptr<bsMesh> mesh(new bsMesh(mMeshCache.getNewMeshId()));

	mFileManager.addAsynchronousLoadRequest(meshName,
		bsMeshCreatorFileLoadFinished(mesh, meshName, *this));

	return mesh;
}

std::shared_ptr<bsMesh> bsMeshCreator::loadMeshSynchronous(const std::string& meshName)
{
	bsSerializedMesh serializedMesh;

	const bool success = bsLoadSerializedMesh(meshName, serializedMesh);
	BS_ASSERT2(success, "Failed to load serialized mesh");

	if (!success)
	{
		//bsMeshSerializer has logged something, just return
		return nullptr;
	}

	bsLog::logf(bsLog::SEV_INFO, "Loaded mesh '%s'", meshName.c_str());

	std::shared_ptr<bsMesh> mesh(constructMeshFromSerializedMesh(serializedMesh, meshName));

	return mesh;
}

std::shared_ptr<bsMesh> bsMeshCreator::constructMeshFromSerializedMesh(
	const bsSerializedMesh& serializedMesh, const std::string& meshName) const
{
	const unsigned int meshCount = serializedMesh.bufferCount;
	std::vector<ID3D11Buffer*> vertexBuffers(meshCount);
	std::vector<ID3D11Buffer*> indexBuffers(meshCount);
	std::vector<unsigned int>  indexCounts(meshCount);
	std::vector<unsigned int>  vertexCounts(meshCount);

	//Create buffers and check for failure for each mesh
	for (unsigned int i = 0; i < meshCount; ++i)
	{
		if (!createBuffers(vertexBuffers[i], indexBuffers[i], meshName, i, serializedMesh))
		{
			std::string errorMessage("Failed to create buffers when loading \'");
			errorMessage.append(meshName);
			errorMessage.append("\'");
			bsLog::logf(bsLog::SEV_ERROR, "Failed to create buffers when loading '%s'",
				meshName.c_str());

			return nullptr;
		}

		indexCounts[i] = serializedMesh.indexBuffers[i].indexCount;
		vertexCounts[i] = serializedMesh.vertexBuffers[i].vertexCount;
	}

	bsCollision::Sphere boundingSphere;
	boundingSphere.positionAndRadius = XMLoadFloat4(&serializedMesh.boundingSphereCenterAndRadius);

	return std::shared_ptr<bsMesh>(new bsMesh(mMeshCache.getNewMeshId(),
		std::move(vertexBuffers), std::move(indexBuffers), std::move(indexCounts),
		std::move(vertexCounts), boundingSphere));
}

bool bsMeshCreator::createBuffers(ID3D11Buffer*& vertexBuffer, ID3D11Buffer*& indexBuffer,
	const std::string& meshName, unsigned int meshIndex,
	const bsSerializedMesh& serializedMesh) const
{
	BS_ASSERT(meshIndex <= serializedMesh.bufferCount);
	//This assert avoids unused formal parameter warning in non-debug builds.
	BS_ASSERT(!meshName.empty());

	const bsVertexBuffer& currentVertexBuffer = serializedMesh.vertexBuffers[meshIndex];
	const bsIndexBuffer& currentIndexBuffer = serializedMesh.indexBuffers[meshIndex];

	//Vertex buffer
	D3D11_BUFFER_DESC bufferDescription;
	memset(&bufferDescription, 0, sizeof(bufferDescription));
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(bsVertexNormalTex)
		* currentVertexBuffer.vertexCount;
	bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData;
	memset(&initData, 0, sizeof(initData));
	initData.pSysMem = currentVertexBuffer.vertices;

	if (FAILED(mD3dDevice->CreateBuffer(&bufferDescription, &initData,
		&vertexBuffer)))
	{
		BS_ASSERT(!"Failed to create vertex buffer, aborting mesh creation");

		return false;
	}

	//Index buffer
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(unsigned int) * currentIndexBuffer.indexCount;
	bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	initData.pSysMem = currentIndexBuffer.indices;

	if (FAILED(mD3dDevice->CreateBuffer(&bufferDescription, &initData,
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
