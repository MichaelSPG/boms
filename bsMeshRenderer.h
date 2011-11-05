#pragma once

#include <memory>

#include "bsMaterial.h"

class bsDx11Renderer;
class bsTexture2D;
class bsMesh;

typedef std::shared_ptr<bsMesh> bsSharedMesh;


class bsMeshRenderer
{
public:
	/*	Create a mesh renderer with a mesh, diffuse texture, and optionally pixel/vertex
		shaders.
		If no pixel/vertex shaders are provided, defaults will be used for opaque geometry.
	*/
	bsMeshRenderer(const bsSharedMesh& mesh, const std::shared_ptr<bsTexture2D>& texture,
		const std::shared_ptr<bsPixelShader>& pixelShader = nullptr,
		const std::shared_ptr<bsVertexShader>& vertexShader = nullptr);


	
	void draw(bsDx11Renderer* dx11Renderer) const;

	void drawInstanced(ID3D11DeviceContext& deviceContext, ID3D11Buffer* instanceBuffer,
		unsigned int instanceCount) const;


	
	inline void setMesh(const bsSharedMesh& mesh)
	{
		mMesh = mesh;
	}

	inline const bsSharedMesh& getMesh() const
	{
		return mMesh;
	}

	inline void setMaterial(const bsMaterial& material)
	{
		mMaterial = material;
	}

	inline const bsMaterial& getMaterial() const
	{
		return mMaterial;
	}

	inline bsMaterial& getMaterial()
	{
		return mMaterial;
	}

	//Functions to forward data from bsMesh.
	bool hasFinishedLoading() const;

	unsigned int getTriangleCount() const;



private:
	bsSharedMesh	mMesh;
	bsMaterial		mMaterial;
};
