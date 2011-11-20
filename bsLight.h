#pragma once

#include <memory>

#include <Windows.h>
#include <xnamath.h>

#include "bsCollision.h"

class bsMesh;
class bsMeshCache;
class bsDx11Renderer;
struct LightInstanceData;


struct bsLightData
{
	//Position of the light.
	//XMFLOAT3	position;
	//Direction of the light (ignored for point lights).
	XMFLOAT3	direction;
	//Diffuse color of the light.
	XMFLOAT3	color;
	//Radius of the light (ignored for directinoal lights).
	float		radius;
	//Controls the spot light cone (only for spot lights).
	float		spotCone;
	//Intensity of the light.
	float		intensity;
	//Attenuation (ignored for directional lights).
	XMFLOAT3	attenuation;
};


/*	This class represents a dynamic light source.

	Must be attached to an entity in order to be active.
	
	This class currently lacks functionality and will be extended in the future.
*/
__declspec(align(16)) class bsLight
{
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
		const bsLightData& cInfo);


	
	/*	Draws light using instancing.
	*/
	void drawInstanced(ID3D11DeviceContext& deviceContext, ID3D11Buffer* instanceBuffer,
		unsigned int instanceCount) const;

	inline LightType getLightType() const
	{
		return mLightType;
	}

	const bsLightData& getLightData() const
	{
		return mLightData;
	}

	inline const bsCollision::Sphere& getBoundingSphere() const
	{
		return mBoundingSphere;
	}


private:
	LightType	mLightType;

	bsLightData	mLightData;

	//Direction for directional and spot lights only
	//XMFLOAT3	mDirection;
	//XMFLOAT3	mColor;
	//
	////Radius for point lights only
	//float		mRadius;
	//float		mIntensity;

	std::shared_ptr<bsMesh>	mMesh;

	bsCollision::Sphere mBoundingSphere;
};
