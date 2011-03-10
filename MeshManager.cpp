#include "MeshManager.h"

#include <sstream>

#include <assimp.hpp>
#include <aiScene.h>
#include <aiPostProcess.h>

#include "Log.h"
#include "Dx11Renderer.h"
#include "VertexTypes.h"



MeshManager::MeshManager(Dx11Renderer* dx11Renderer)
	: mNumCreatedMeshes(0u)
	, mDx11Renderer(dx11Renderer)
{
	assert(dx11Renderer);
}

MeshManager::~MeshManager()
{

}

const std::shared_ptr<Mesh> MeshManager::getMesh(const std::string& meshName)
{
	assert(meshName.length());

	//See if the mesh already exists
	auto val = mMeshes.find(meshName);
	if (val != mMeshes.end())
	{
		return val->second;
	}

	//Not found, create and return the shader
	std::shared_ptr<Mesh> mesh = createMesh(meshName);
	if (!mesh)
	{
		std::stringstream message;
		message << "Something went wrong while creating '" << meshName << '\'';
		Log::logMessage(message.str().c_str(), pantheios::SEV_ERROR);

		return nullptr;
	}

	return mesh;
}

std::shared_ptr<Mesh> MeshManager::createMesh(const std::string& meshName)
{
	std::stringstream debugMessage;
	debugMessage << "Starting mesh creation of '" << meshName << '\'';
	Log::logMessage(debugMessage.str().c_str(), pantheios::SEV_DEBUG);

	Assimp::Importer importer;
	unsigned int flags = aiProcess_Triangulate;
#ifdef _DEBUG
	flags |= aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded;
#else
	flags |= aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded;
#endif // _DEBUG

	const aiScene* scene = importer.ReadFile(meshName.c_str(), flags);
	if (!scene)
	{
		Log::logMessage(importer.GetErrorString(), pantheios::SEV_ERROR);

		return nullptr;
	}

	unsigned int meshCount = scene->mNumMeshes;
	if (meshCount == 0)
	{
		std::stringstream message;
		message << '\'' << meshName << "' contains no meshes";
		Log::logMessage(message.str().c_str(), pantheios::SEV_ERROR);

		return nullptr;
	}

	std::vector<std::vector<VertexNormalTex>> vertices(meshCount);
	std::vector<std::vector<unsigned int>> indices(meshCount);

	for (unsigned int i = 0u; i < meshCount; ++i)
	{
		parseData(vertices[i], indices[i], scene->mMeshes[i]);
		//parseData(mesh->mSubMeshes[i]->)
	}
	//parseData(vertices, indices, scene);

	std::vector<ID3D11Buffer*> vertexBuffers(meshCount);
	std::vector<ID3D11Buffer*> indexBuffers(meshCount);

	//Create the buffers from the vertex and index buffers for each mesh
	for (unsigned int i = 0u; i < meshCount; ++i)
	{
		if (!createBuffers(vertices[i], indices[i], vertexBuffers[i], indexBuffers[i], meshName))
		{
			std::stringstream message;
			message << "Failed to create buffers from '" << meshName << '\'';
			Log::logMessage(message.str().c_str(), pantheios::SEV_ERROR);

			return nullptr;
		}
	}

	assert(vertexBuffers.size() && indexBuffers.size());

	std::stringstream message;
	message << "Created mesh '" << meshName << "'";
	Log::logMessage(message.str().c_str());

	Mesh* mesh = new Mesh();
	mesh->mID = getNumCreatedMeshes();
	mesh->mSubMeshes.resize(meshCount);

	//Create all the submeshes and set their values
	for (unsigned int i = 0u; i < meshCount; ++i)
	{
		mesh->mSubMeshes[i] = new Mesh();
		mesh->mSubMeshes[i]->mID = getNumCreatedMeshes();
		mesh->mSubMeshes[i]->mIndexBuffer = indexBuffers[i];
		mesh->mSubMeshes[i]->mVertexBuffer = vertexBuffers[i];
		mesh->mSubMeshes[i]->mIndices = indices[i].size();
	}

//	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(getNumCreatedMeshes(),
//		vertexBuffer, indexBuffer, indices.size());
	std::shared_ptr<Mesh> spMesh(mesh);
	mMeshes[meshName] = spMesh;
	
	return spMesh;
}

void MeshManager::parseData(std::vector<VertexNormalTex>& vertices,
	std::vector<unsigned int>& indices, const aiMesh* mesh)
{
	bool meshHasNormals = mesh->HasNormals();

	for (unsigned int j = 0u; j < mesh->mNumFaces; ++j)
	{
		aiFace face = mesh->mFaces[j];

		//assert(face.mNumIndices == 3 && "Faces with more/less than 3 indices is not supported");
		for (unsigned int k = 0; k < face.mNumIndices; ++k)
		{
			indices.push_back(face.mIndices[k]);
		}
		/*
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
		*/
	}

	for (unsigned int j = 0u; j < mesh->mNumVertices; ++j)
	{
		VertexNormalTex vertex;
		vertex.position = mesh->mVertices[j];
		if (meshHasNormals)
		{
			vertex.normal = mesh->mNormals[j];
		}
		vertices.push_back(vertex);
	}

	//Make sure something has been loaded.
	assert(vertices.size() && indices.size());
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
