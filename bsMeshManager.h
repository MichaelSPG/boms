#ifndef BS_MESHMANAGER_H
#define BS_MESHMANAGER_H

#include "bsConfig.h"

#include <map>
#include <memory>
#include <string>

#include "bsMesh.h"
#include "bsVertexTypes.h"

class bsResourceManager;
class bsDx11Renderer;


/*	The mesh manager keeps track of every loaded mesh, and it loads meshes.
	The meshes are stored in a map with the file path as the key, making it possible to
	easily return a pointer to a mesh that has already been loaded if it exists in the map.
*/
class bsMeshManager
{
public:
	bsMeshManager(bsDx11Renderer* dx11Renderer, bsResourceManager* resourceManager);

	~bsMeshManager();

	/*	Get a shared pointer to a mesh.
		This will look for the mesh name in the known resource locations and load it if it
		has not already bene loaded.
	*/
	std::shared_ptr<bsMesh> getMesh(const std::string& meshName) const;
	

private:
	//Loads a mesh from disk.
	std::shared_ptr<bsMesh> loadMesh(const std::string& meshName);

	/*	Creates the buffers for a mesh, allowing it to be uploaded to the GPU and rendered.
		Returns true on success.
	*/
	bool createBuffers(const std::vector<VertexNormalTex>& vertices,
		const std::vector<unsigned int>& indices, ID3D11Buffer*& vertexBuffer,
		ID3D11Buffer*& indexBuffer, const std::string& meshName);

	//Increments loaded mesh count and returns the value.
	inline unsigned int getNewMeshId()
	{
		return ++mNumLoadedMeshes;
	}

	std::map<std::string, std::shared_ptr<bsMesh>>	mMeshes;

	bsResourceManager*	mResourceManager;
	bsDx11Renderer*		mDx11Renderer;
	unsigned int		mNumLoadedMeshes;
};

#endif // BS_MESHMANAGER_H
