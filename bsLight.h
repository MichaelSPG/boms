#ifndef BS_LIGHT_H
#define BS_LIGHT_H

#include "bsConfig.h"

#include <memory>

#include <Windows.h>
#include <xnamath.h>

#include "bsRenderable.h"

class bsMesh;
class bsMeshManager;
class bsDx11Renderer;

struct bsPointLightCInfo
{
	XMFLOAT3	color;
	float		radius;
	float		intensity;
};

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

	bsLight(LightType lightType, bsMeshManager* meshManager,
		const bsPointLightCInfo& cInfo);

	~bsLight();


	inline RenderableType getRenderableType() const
	{
		return LIGHT;
	}

	virtual bool isOkForRendering() const
	{
		return true;
	}

	virtual const hkAabb& getAabb() const
	{
		return mAabb;
	}

	void draw(bsDx11Renderer* dx11Renderer) const;

protected:
	std::shared_ptr<bsMesh>	mMesh;
	hkAabb					mAabb;
private:

	LightType	mLightType;

	//Direction for directional and spot lights only
	XMFLOAT3	mDirection;
	XMFLOAT3	mColor;

	//Radius for point lights only
	float		mRadius;
	float		mIntensity;
};

#endif // BS_LIGHT_H