#include "bsLight.h"

#include "bsMeshManager.h"
#include "bsDx11Renderer.h"


bsLight::bsLight(LightType lightType, bsMeshManager* meshManager,
	const bsPointLightCInfo& cInfo)
	: mMesh(meshManager->getMesh("sphere_1m_d.bsm"))
	, mColor(cInfo.color)
	, mRadius(cInfo.radius)
	, mIntensity(cInfo.intensity)
{
	memset(&mDirection, 0, sizeof(mDirection));

	//TODO: Set this to equal parameter
	mLightType = LT_POINT;

	const float halfRadius = mRadius * 0.5f;
	mAabb.m_min.setAll3(-halfRadius);
	mAabb.m_max.setAll3(halfRadius);
}

bsLight::~bsLight()
{
	
}

void bsLight::draw(bsDx11Renderer* dx11Renderer) const
{
	mMesh->draw(dx11Renderer);
}
