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

class bsFileSystem;
class bsFileIoManager;


/*	The mesh manager keeps track of every loaded mesh, and it loads meshes.
	The meshes are stored in a map with the file path as the key, making it possible to
	easily return a pointer to a mesh that has already been loaded if it exists in the map.
*/
class bsMeshCache
{
public:
	bsMeshCache(bsDx11Renderer* dx11Renderer, const bsFileSystem& fileSystem,
		bsFileIoManager& fileIoManager);

	~bsMeshCache();

	/*	Get a shared pointer to a mesh.
		This will look for the mesh name in the known resource locations and load it if it
		has not already bene loaded.
	*/
	std::shared_ptr<bsMesh> getMesh(const std::string& meshName) const;

	/*	Loads a mesh from disk asynchronously.
		This function will be called automatically by getMesh if the requested mesh has not
		already been loaded, but it is also possible to call it manually if preferred.

		Attempting to load an already load a mesh may result in memory leaks and internal
		corruption.
	*/
	std::shared_ptr<bsMesh> loadMesh(const std::string& meshName);

	/*	Loads a mesh from disk, but unlike loadMesh, this function blocks until the mesh
		has completed loading.

		This function can be used to ensure that a mesh exists before rendering starts or
		similar, but calling it during rendering may cause stuttering.

		Attempting to load an already load a mesh may result in memory leaks and internal
		corruption.
	*/
	std::shared_ptr<bsMesh> loadMeshBlocking(const std::string& meshName);



	
	/*	Used for giving new meshes unique IDs.
		For internal use only.
	*/
	inline unsigned int getNewMeshId()
	{
		return ++mNumLoadedMeshes;
	}


private:
	/*	Verifies that the mesh specified by the parameter has not already been loaded.
		Returns true if the mesh is not in the cache.
	*/
	inline bool verifyMeshIsNotAlreadyInCache(const std::string& meshName);

	/*	Verifies that the mesh path is valid, ie not empty.
		Returns true if the path is valid.
	*/
	inline bool verifyMeshPathIsValid(const std::string& meshPath,
		const std::string& meshName);


	std::unordered_map<std::string, std::shared_ptr<bsMesh>>	mMeshes;

	const bsFileSystem&	mFileSystem;
	bsFileIoManager&	mFileIoManager;
	unsigned int		mNumLoadedMeshes;

	bsMeshCreator	mMeshCreator;
};
