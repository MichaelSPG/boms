#pragma once

#include "bsConfig.h"

#include <string>
#include <vector>
#include <memory>

#include <Windows.h>
#include <d3d11.h>
#include <D3DX11.h>

#include <Common/Base/hkBase.h>

#include "bsRenderable.h"

class bsDx11Renderer;


/*	Class for mesh rendering.
	Contains vertex and index buffers for a mesh, and possibly a collection of sub-meshes.
	Use the bsMeshCache class to create meshes.

	This mesh class works like a tree. There is one root mesh with N sub meshes. The root
	mesh has no index/vertex buffers on its own.
	Only the root mesh has sub meshes, the sub meshes have no sub meshes, so the tree depth
	is never greater than 2.
*/
class bsMesh : public bsRenderable
{
	friend class bsMeshCache;
	friend class bsSceneNode;
	friend class bsRenderQueue;

public:
	//For container purposes only, do not use this constructor.
	inline bsMesh()
		: mVertexBuffer(nullptr)
		, mIndexBuffer(nullptr)
	{}

	//Initialize a mesh with data created from bsMeshCreator.
	bsMesh(unsigned int id, unsigned int numSubMeshes, ID3D11Buffer* vertexBuffer,
		ID3D11Buffer* indexBuffer, unsigned int indices);

	~bsMesh();

	bsMesh(bsMesh&& other);

	bsMesh& operator=(bsMesh&& other);

	void draw(bsDx11Renderer* dx11Renderer) const;

	/*	Returns the collection of sub-meshes owned by this mesh.
		This may contain zero elements if this mesh has no sub-meshes.
	*/
	inline const std::vector<bsMesh>& getSubMeshes() const
	{
		return mSubMeshes;
	}

	inline std::vector<bsMesh>& getSubMeshes()
	{
		return mSubMeshes;
	}

	inline RenderableType getRenderableType() const
	{
		return MESH;
	}

	inline bool isOkForRendering() const
	{
		return mFinished;
	}

	//Set min and max point of this mesh' AABB. Also calls updateAabb to keep AABB up to date.
	void setAabb(const hkVector4& min, const hkVector4& max);

	//Updates this mesh' AABB so that it contains all of this mesh' sub-meshes' AABBs.
	void updateAabb();


private:
	//Not copyable
	bsMesh(const bsMesh&);
	void operator=(const bsMesh&);


	ID3D11Buffer*		mVertexBuffer;
	ID3D11Buffer*		mIndexBuffer;
	unsigned int		mIndices;
	std::vector<bsMesh>	mSubMeshes;

	unsigned int	mID;

	bool			mFinished;
};
