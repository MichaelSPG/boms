#pragma once


#include <memory>

#include <Windows.h>
#include <xnamath.h>

#include <Common/Base/hkBase.h>

#include "bsRenderable.h"

class bsMesh;
class bsMeshCache;
class bsDx11Renderer;


struct bsPointLightCInfo
{
	XMFLOAT3	color;
	float		radius;
	float		intensity;
};


/*	This class represents a dynamic light source.

	Must be attached to a scene node in order to illuminate objects.
	
	This class currently lacks functionality and will be extended in the future.
*/
class bsLight : public bsRenderable
{
	friend class bsRenderQueue;

public:
	enum LightType
	{
		LT_DIRECTIONAL,
		LT_POINT,
		LT_SPOT
	};

	bsLight(LightType lightType, bsMeshCache* meshCache,
		const bsPointLightCInfo& cInfo);

	~bsLight();

	/*	Returns LIGHT.
	*/
	inline RenderableType getRenderableType() const
	{
		return LIGHT;
	}

	virtual bool hasFinishedLoading() const
	{
		return true;
	}

	/*	Draws the mesh this light uses to represent itself.
	*/
	void draw(bsDx11Renderer* dx11Renderer) const;

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
};
