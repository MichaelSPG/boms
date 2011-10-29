#pragma once

#include <memory>

#include <Windows.h>
#include <xnamath.h>

#include "bsCollision.h"

class bsMesh;
class bsMeshCache;
class bsDx11Renderer;
struct LightInstanceData;


struct bsPointLightCInfo
{
	XMFLOAT3	color;
	float		radius;
	float		intensity;
};


/*	This class represents a dynamic light source.

	Must be attached to an entity in order to be active.
	
	This class currently lacks functionality and will be extended in the future.
*/
__declspec(align(16)) class bsLight
{
	friend class bsRenderQueue;

public:
	enum LightType
	{
		LT_DIRECTIONAL,
		LT_POINT,
		LT_SPOT
	};

	inline void* operator new(size_t)
	{
		return _aligned_malloc(sizeof(bsLight), 16);
	}
	inline void operator delete(void* p)
	{
		_aligned_free(p);
	}


	bsLight(LightType lightType, bsMeshCache* meshCache,
		const bsPointLightCInfo& cInfo);


	/*	Draws the mesh this light uses to represent itself.
	*/
	void draw(bsDx11Renderer* dx11Renderer) const;

	void drawInstanced(ID3D11DeviceContext& deviceContext, ID3D11Buffer* instanceBuffer,
		unsigned int instanceCount) const;

	inline float getRadius() const
	{
		return mRadius;
	}

	inline const bsCollision::Sphere& getBoundingSphere() const
	{
		return mBoundingSphere;
	}


protected:
	std::shared_ptr<bsMesh>	mMesh;
private:

	LightType	mLightType;

	//Direction for directional and spot lights only
	XMFLOAT3	mDirection;
	XMFLOAT3	mColor;

	//Radius for point lights only
	float		mRadius;
	float		mIntensity;

	bsCollision::Sphere mBoundingSphere;
};
