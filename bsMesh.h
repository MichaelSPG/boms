#ifndef MESH_H
#define MESH_H

#include <memory>
#include <string>

#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>
#include <Common/Base/Types/Geometry/Aabb/hkAabbUtil.h>

#include "bsRenderable.h"

#include "bsDx11Renderer.h"
#include "bsPrimitive.h"

class bsShaderManager;
class bsVertexShader;
class bsPixelShader;


class bsMesh : public bsRenderable
{
	friend class bsMeshManager;
	friend class bsSceneNode;
	friend class bsRenderQueue;

	bsMesh();

public:
	~bsMesh();

	void draw(bsDx11Renderer* dx11Renderer) const;

	void createDrawableAabb(bsDx11Renderer* dx11Renderer, bsShaderManager* shaderManager);

	inline std::vector<bsMesh*> getSubMeshes()
	{
		return mSubMeshes;
	}

	//nullptr if not created
	inline bsPrimitive* getDrawablePrimitive()
	{
		return mAabbPrimitive;
	}

	inline const RenderableIdentifier getRenderableIdentifier() const
	{
		return MESH;
	}

	bool operator<(const bsMesh& other);

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
	
	std::shared_ptr<bsVertexShader>	mVertexShader;
	std::shared_ptr<bsPixelShader>	mPixelShader;

	hkAabb			mAabb;
	unsigned int	mID;
	std::string	mName;

	bool			mDrawableAabbActive;
	bsPrimitive*	mAabbPrimitive;
};

#endif // MESH_H
