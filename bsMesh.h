#ifndef MESH_H
#define MESH_H

#include "bsConfig.h"

#include <string>
#include <vector>

#include <Windows.h>
#include <d3d11.h>
#include <D3DX11.h>

#include "bsRenderable.h"

class bsDx11Renderer;


class bsMesh : public bsRenderable
{
	friend class bsMeshManager;
	friend class bsSceneNode;
	friend class bsRenderQueue;

	bsMesh();

public:
	~bsMesh();

	void draw(bsDx11Renderer* dx11Renderer) const;

	inline std::vector<bsMesh*> getSubMeshes()
	{
		return mSubMeshes;
	}

	inline RenderableIdentifier getRenderableIdentifier() const
	{
		return MESH;
	}

	bool operator<(const bsMesh& other);

	bool isOkForRendering() const
	{
		return mFinished;
	}

private:
	//Updates this mesh' AABB so that it contains all of this mesh' submeshes' AABBs.
	void updateAABB();

	//Not copyable
	bsMesh(const bsMesh&);
	void operator=(const bsMesh&);

	ID3D11Buffer*		mVertexBuffer;
	ID3D11Buffer*		mIndexBuffer;
	unsigned int		mIndices;
	std::vector<bsMesh*>	mSubMeshes;

	unsigned int	mID;
#if BS_DEBUG_LEVEL > 1
	//Contains the mesh' file name. For debugging purposes only, never rely on this
	//variable existing.
	std::string		mName;
#endif

	bool			mFinished;
};

#endif // MESH_H
