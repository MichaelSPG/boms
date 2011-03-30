#ifndef MESHMANAGER_H
#define MESHMANAGER_H

#include "bsConfig.h"

#include <map>
#include <memory>
#include <string>

#include "bsMesh.h"

class bsResourceManager;
class bsDx11Renderer;


class bsMeshManager
{
public:
	bsMeshManager(bsDx11Renderer* dx11Renderer, bsResourceManager* resourceManager);
	~bsMeshManager();

	std::shared_ptr<bsMesh> getMesh(const std::string& meshName);
	

private:
	//Loads a mesh created with the mesh serializer
	std::shared_ptr<bsMesh> loadMesh(const std::string& meshName);

	std::shared_ptr<bsMesh> createMesh(const std::string& meshName);

	//Returns true on success.
	bool createBuffers(const std::vector<VertexNormalTex>& vertices,
		const std::vector<unsigned int>& indices, ID3D11Buffer*& vertexBuffer,
		ID3D11Buffer*& indexBuffer, const std::string& meshName);

	inline unsigned int getNumCreatedMeshes()
	{
		return ++mNumCreatedMeshes;
	}

	std::map<std::string, std::shared_ptr<bsMesh>>	mMeshes;

	bsResourceManager*	mResourceManager;
	bsDx11Renderer*		mDx11Renderer;
	unsigned int		mNumCreatedMeshes;
};

#endif // MESHMANAGER_H