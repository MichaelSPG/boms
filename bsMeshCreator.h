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


class bsMeshCreator
{
public:
	bsMeshCreator(bsMeshCache& meshCache, bsDx11Renderer& dx11Renderer);

	~bsMeshCreator();


	//Loads a mesh from the hard drive, creates vertex and index buffers for it and returns it.
	std::shared_ptr<bsMesh> loadMesh(const std::string& meshName);

private:
	//Non-copyable.
	bsMeshCreator& operator=(const bsMeshCreator&);
	bsMeshCreator(const bsMeshCreator&);

	/*	Creates the buffers for a mesh, allowing it to be uploaded to the GPU and rendered.
		Returns true on success.
	*/
	bool createBuffers(ID3D11Buffer*& vertexBuffer, ID3D11Buffer*& indexBuffer,
		const std::string& meshName, unsigned int meshIndex);


	bsMeshCache&		mMeshCache;
	bsDx11Renderer&		mDx11Renderer;
	bsSerializedMesh	mSerializedMesh;
};
