#ifndef MESHMANAGER_H
#define MESHMANAGER_H

#include "bsConfig.h"

#include <map>
#include <memory>
#include <string>

#include "Mesh.h"

class ResourceManager;
class Dx11Renderer;


class MeshManager
{
public:
	MeshManager(Dx11Renderer* dx11Renderer, ResourceManager* resourceManager);
	~MeshManager();

	std::shared_ptr<Mesh> getMesh(const std::string& meshName);
	

private:
	//Loads a mesh created with the mesh serializer
	std::shared_ptr<Mesh> loadMesh(const std::string& meshName);

	std::shared_ptr<Mesh> createMesh(const std::string& meshName);

	//Returns true on success.
	bool createBuffers(const std::vector<VertexNormalTex>& vertices,
		const std::vector<unsigned int>& indices, ID3D11Buffer*& vertexBuffer,
		ID3D11Buffer*& indexBuffer, const std::string& meshName);

	inline unsigned int getNumCreatedMeshes()
	{
		return ++mNumCreatedMeshes;
	}

	std::map<std::string, std::shared_ptr<Mesh>>	mMeshes;

	ResourceManager*	mResourceManager;
	Dx11Renderer*		mDx11Renderer;
	unsigned int		mNumCreatedMeshes;
};

#endif // MESHMANAGER_H