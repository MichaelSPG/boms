#include "StdAfx.h"

#include "bsMeshCache.h"

#include <d3d11.h>
#include <D3DX11.h>

#include "bsResourceManager.h"
#include "bsLog.h"
#include "bsDx11Renderer.h"
#include "bsMeshSerializer.h"
#include "bsAssert.h"


#pragma warning(push)
//The mesh creator's constructor does not use this for anything but a reference assignment,
//and this does not have a vtable, so no undefined/unsafe behavior is occuring.
#pragma warning(disable:4355)// warning C4355: 'this' : used in base member initializer list

bsMeshCache::bsMeshCache(bsDx11Renderer* dx11Renderer, const bsFileSystem& fileSystem,
	bsFileIoManager& fileIoManager)
	: mNumLoadedMeshes(0)
	, mFileSystem(fileSystem)
	, mFileIoManager(fileIoManager)
	, mMeshCreator(*this, *dx11Renderer, fileSystem, mFileIoManager)
{
	BS_ASSERT(dx11Renderer);
}

#pragma warning(pop)

bsMeshCache::~bsMeshCache()
{
	//TODO: Maybe remove this
	//Check that there are no meshes being referenced elsewhere when the mesh manager is
	//shut down.

#ifdef BS_DEBUG
	for (auto itr = mMeshes.begin(), end = mMeshes.end(); itr != end; ++itr)
	{
		if (!(itr->second.use_count() == 1))
		{
			bsLog::log("There are external references to meshes when the mesh"
				"manager is shutting down", bsLog::SEV_WARNING);

			break;
		}
	}
#endif //BS_DEBUG
}

std::shared_ptr<bsMesh> bsMeshCache::getMesh(const std::string& meshName) const
{
	BS_ASSERT2(meshName.length(), "Zero length file names are not OK");

	//See if the mesh already exists
	auto findResult = mMeshes.find(meshName);
	if (findResult != mMeshes.end())
	{
		return findResult->second;
	}

	//Not found, need to create it now.
	//Cast const away since the load function is not const
	std::shared_ptr<bsMesh> mesh(const_cast<bsMeshCache*>(this)->loadMeshAsync(meshName));
	BS_ASSERT2(mesh != nullptr, std::string("Something went wrong while creating \'") + meshName + '\'');

	return mesh;
}

std::shared_ptr<bsMesh> bsMeshCache::loadMeshAsync(const std::string& meshName)
{
#ifdef BS_DEBUG
	if (!verifyMeshIsNotAlreadyInCache(meshName))
	{
		return nullptr;
	}
#endif

	//Get relative path of the mesh file.
	const std::string meshPath(mFileSystem.getPathFromFilename(meshName));
	if (!verifyMeshPathIsValid(meshPath, meshName))
	{
		return nullptr;
	}

	std::shared_ptr<bsMesh> mesh(mMeshCreator.loadMeshAsync(meshPath));

	//Add the mesh to the cache, allowing it to not be reloaded the next time it's requested.
	mMeshes[meshName] = mesh;

	return mesh;
}

std::shared_ptr<bsMesh> bsMeshCache::loadMeshSynchronously(const std::string& meshName)
{
#ifdef BS_DEBUG
	if (!verifyMeshIsNotAlreadyInCache(meshName))
	{
		return nullptr;
	}
#endif

	const std::string meshPath(mFileSystem.getPathFromFilename(meshName));
	if (!verifyMeshPathIsValid(meshPath, meshName))
	{
		return nullptr;
	}

	std::shared_ptr<bsMesh> mesh(mMeshCreator.loadMeshSynchronous(meshPath));
	
	mMeshes[meshName] = mesh;

	return mesh;
}

inline bool bsMeshCache::verifyMeshIsNotAlreadyInCache(const std::string& meshName)
{
	//Verify that the mesh has not already been loaded. This should only happen due to
	//incorrect usage of this class.
	if (mMeshes.find(meshName) != mMeshes.end())
	{
		bsLog::logf(bsLog::SEV_ERROR, "bsMeshCache::loadMeshAsync was called, but the"
			" requested mesh has already been loaded. Mesh name: '%s'", meshName.c_str());
		
		BS_ASSERT2(false, "bsMeshCache::loadMeshAsync was "
			"called, but the requested mesh has already been loaded!");

		return false;
	}

	return true;
}

inline bool bsMeshCache::verifyMeshPathIsValid(const std::string& meshPath,
	const std::string& meshName)
{
	//Verify that the path of the mesh actually exists.
	if (meshPath.empty())
	{
		bsLog::logf(bsLog::SEV_ERROR, "'%s' does not exist in any known resource paths,"
			" it will not be loaded");

		BS_ASSERT2(false, "Failed to load mesh");

		return false;
	}

	return true;
}
