#include "StdAfx.h"

#include "bsLight.h"

#include "bsMeshCache.h"
#include "bsDx11Renderer.h"
#include "bsAssert.h"
#include "bsConstantBuffers.h"


bsLight::bsLight(LightType lightType, bsMeshCache* meshCache,
	const bsPointLightCInfo& cInfo)
	: mLightType(lightType)
	, mMesh(meshCache->getMesh("sphere_1m_d.bsm"))
	, mColor(cInfo.color)
	, mRadius(cInfo.radius)
	, mIntensity(cInfo.intensity)
{
	memset(&mDirection, 0, sizeof(mDirection));

	BS_ASSERT2(lightType == LT_POINT, "Only points lights are functional");

	const float halfRadius = mRadius * 0.5f;
	mAabb.m_min.setAll(-halfRadius);
	mAabb.m_max.setAll(halfRadius);
}

bsLight::~bsLight()
{
	
}

void bsLight::draw(bsDx11Renderer* dx11Renderer) const
{
	mMesh->draw(dx11Renderer);
}

void bsLight::drawInstanced(ID3D11DeviceContext& deviceContext,
	ID3D11Buffer* instanceBuffer, unsigned int instanceCount)
{
	if (!mMesh->hasFinishedLoading())
	{
		return;
	}

	const std::vector<ID3D11Buffer*>& vertexBuffers = mMesh->getVertexBuffers();
	const std::vector<ID3D11Buffer*>& indexBuffers = mMesh->getIndexBuffers();
	const std::vector<unsigned int>& indexCounts = mMesh->getIndexCounts();


	unsigned int strides[2] = { sizeof(bsVertexNormalTex), sizeof(LightInstanceData) };
	unsigned int offsets[2] = { 0, 0 };
	ID3D11Buffer* vertexInstanceBuffers[2] = { nullptr, instanceBuffer };

	const unsigned int bufferCount = vertexBuffers.size();
	for (unsigned int i = 0; i < bufferCount; ++i)
	{
		vertexInstanceBuffers[0] = vertexBuffers[i];
		deviceContext.IASetVertexBuffers(0, 2, vertexInstanceBuffers, strides, offsets);
		deviceContext.IASetIndexBuffer(indexBuffers[i], DXGI_FORMAT_R32_UINT, 0);

		deviceContext.DrawIndexedInstanced(indexCounts[i], instanceCount, 0, 0, 0);
	}
}
