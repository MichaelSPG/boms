#pragma once

#include "bsConfig.h"

#include <map>
#include <unordered_map>
#include <memory>
#include <string>

#include <Common/Base/hkBase.h>

#include "bsMesh.h"
#include "bsVertexTypes.h"
#include "bsMeshCreator.h"

class bsResourceManager;


/*	The mesh manager keeps track of every loaded mesh, and it loads meshes.
	The meshes are stored in a map with the file path as the key, making it possible to
	easily return a pointer to a mesh that has already been loaded if it exists in the map.
*/
class bsMeshCache
{
public:
	bsMeshCache(bsDx11Renderer* dx11Renderer, bsResourceManager* resourceManager);

	~bsMeshCache();

	/*	Get a shared pointer to a mesh.
		This will look for the mesh name in the known resource locations and load it if it
		has not already bene loaded.
	*/
	std::shared_ptr<bsMesh> getMesh(const std::string& meshName) const;

	/*	Loads a mesh from disk.
		This function will be called automatically by getMesh if the requested mesh has not
		already been loaded, but it is also possible to call it manually if preferred.

		Attempting to load an already load a mesh may result in memory leaks and internal
		corruption.
	*/
	std::shared_ptr<bsMesh> loadMesh(const std::string& meshName);
	
	/*	Increments loaded mesh count and returns the value.
		For internal use only.
	*/
	inline unsigned int getNewMeshId()
	{
		return ++mNumLoadedMeshes;
	}


private:
	std::unordered_map<std::string, std::shared_ptr<bsMesh>>	mMeshes;

	bsResourceManager*	mResourceManager;
	unsigned int		mNumLoadedMeshes;

	bsMeshCreator	mMeshCreator;
};
