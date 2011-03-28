#ifndef MESH_H
#define MESH_H

#include <memory>
#include <string>

#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>
#include <Common/Base/Types/Geometry/Aabb/hkAabbUtil.h>

#include "Renderable.h"

#include "Dx11Renderer.h"
#include "Primitive.h"

class ShaderManager;
class VertexShader;
class PixelShader;


class Mesh : public Renderable
{
	friend class MeshManager;
	friend class SceneNode;
	friend class RenderQueue;

	Mesh();

public:
	~Mesh();

	void draw(Dx11Renderer* dx11Renderer) const;

	void createDrawableAabb(Dx11Renderer* dx11Renderer, ShaderManager* shaderManager);

	inline std::vector<Mesh*> getSubMeshes()
	{
		return mSubMeshes;
	}

	//nullptr if not created
	inline Primitive* getDrawablePrimitive()
	{
		return mAabbPrimitive;
	}

	inline const RenderableIdentifier getRenderableIdentifier() const
	{
		return MESH;
	}

	bool operator<(const Mesh& other);

private:
	//Updates this mesh' AABB so that it contains all of this mesh' submeshes' AABBs.
	void updateAABB();

	//Not copyable
	Mesh(const Mesh&);
	void operator=(const Mesh&);

	ID3D11Buffer*		mVertexBuffer;
	ID3D11Buffer*		mIndexBuffer;
	unsigned int		mIndices;
	std::vector<Mesh*>	mSubMeshes;
	
	std::shared_ptr<VertexShader>	mVertexShader;
	std::shared_ptr<PixelShader>	mPixelShader;

	hkAabb			mAabb;
	unsigned int	mID;
	std::string	mName;

	bool		mDrawableAabbActive;
	Primitive*	mAabbPrimitive;
};

#endif // MESH_H