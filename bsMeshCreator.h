#pragma once

#include <string>
#include <memory>
#include <vector>

#include <d3d11.h>

#include <Common/Base/hkBase.h>

#include "bsVertexTypes.h"
#include "bsMeshSerializer.h"

class bsMesh;
class bsMeshCache;
class bsDx11Renderer;
class bsFileSystem;
class bsFileLoader;
class bsFileIoManager;
struct ID3D11Device;


class bsMeshCreator
{
	friend class bsMeshCreatorFileLoadFinished;

public:
	bsMeshCreator(bsMeshCache& meshCache, const bsDx11Renderer& dx11Renderer,
		const bsFileSystem& fileSystem, bsFileIoManager& fileManager);

	~bsMeshCreator();


	/*	Loads and constructs a mesh from disk asynchronously.
	*/
	std::shared_ptr<bsMesh> loadMeshAsync(const std::string& meshName);

	/*	Loads and constructs a mesh from disk and blocks until the operation completes.
	*/
	std::shared_ptr<bsMesh> loadMeshSynchronous(const std::string& meshName);


private:
	//Non-copyable.
	bsMeshCreator& operator=(const bsMeshCreator&);
	bsMeshCreator(const bsMeshCreator&);

	std::shared_ptr<bsMesh> constructMeshFromSerializedMesh(
		const bsSerializedMesh& serializedMesh, const std::string& meshName) const;

	/*	Creates the buffers for a mesh, allowing it to be uploaded to the GPU and rendered.
		Returns true on success.
	*/
	bool createBuffers(ID3D11Buffer*& vertexBuffer, ID3D11Buffer*& indexBuffer,
		const std::string& meshName, unsigned int meshIndex,
		const bsSerializedMesh& serializedMesh) const;


	bsMeshCache&		mMeshCache;
	ID3D11Device*		mD3dDevice;
	const bsFileSystem&	mFileSystem;
	bsFileIoManager&	mFileManager;
};
