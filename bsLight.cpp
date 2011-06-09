#include "bsLight.h"

#include "bsMeshManager.h"
#include "bsDx11Renderer.h"
#include "bsAssert.h"


bsLight::bsLight(LightType lightType, bsMeshManager* meshManager,
	const bsPointLightCInfo& cInfo)
	: mLightType(lightType)
	, mMesh(meshManager->getMesh("sphere_1m_d.bsm"))
	, mColor(cInfo.color)
	, mRadius(cInfo.radius)
	, mIntensity(cInfo.intensity)
{
	memset(&mDirection, 0, sizeof(mDirection));

	BS_ASSERT2(lightType == LT_POINT, "Only points lights are functional");

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
