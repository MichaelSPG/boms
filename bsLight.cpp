#include "bsLight.h"

#include "bsMeshManager.h"
#include "bsDx11Renderer.h"


bsLight::bsLight(LightType lightType, bsMeshManager* meshManager,
	const bsPointLightCInfo& cInfo)
	: mMesh(meshManager->getMesh("sphere_1m_d.bsm"))
	, mAabb(mMesh->getAabb())
	, mColor(cInfo.color)
	, mRadius(cInfo.radius)
	, mIntensity(cInfo.intensity)
{
	memset(&mDirection, 0, sizeof(mDirection));

	//TODO: Set this to equal parameter
	mLightType = LT_POINT;

	mAabb.expandBy(mRadius);
}

bsLight::~bsLight()
{
	
}

void bsLight::draw(bsDx11Renderer* dx11Renderer) const
{
	mMesh->draw(dx11Renderer);
}
