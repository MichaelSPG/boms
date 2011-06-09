#ifndef BS_MESH_H
#define BS_MESH_H

#include "bsConfig.h"

#include <string>
#include <vector>
#include <memory>

#include <Windows.h>
#include <d3d11.h>
#include <D3DX11.h>

#include "bsRenderable.h"

class bsDx11Renderer;


/*	Class for mesh rendering.
	Contains vertex and index buffers for a mesh, and possibly a collection of sub-meshes.
	Use the bsMeshManager class to create meshes.
*/
class bsMesh : public bsRenderable
{
	friend class bsMeshManager;
	friend class bsSceneNode;
	friend class bsRenderQueue;

public:
	bsMesh();

	~bsMesh();

	void draw(bsDx11Renderer* dx11Renderer) const;

	/*	Returns the collection of sub-meshes owned by this mesh.
		This may contain zero elements if this mesh has no sub-meshes.
	*/
	inline const std::vector<bsMesh*>& getSubMeshes() const
	{
		return mSubMeshes;
	}

	inline RenderableType getRenderableType() const
	{
		return MESH;
	}

	bool isOkForRendering() const
	{
		return mFinished;
	}

private:
	//Updates this mesh' AABB so that it contains all of this mesh' sub-meshes' AABBs.
	void updateAABB();

	//Not copyable
	bsMesh(const bsMesh&);
	void operator=(const bsMesh&);

	ID3D11Buffer*		mVertexBuffer;
	ID3D11Buffer*		mIndexBuffer;
	unsigned int		mIndices;
	std::vector<bsMesh*>	mSubMeshes;

	unsigned int	mID;

	bool			mFinished;
};

#endif // BS_MESH_H
