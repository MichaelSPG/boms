#include "StdAfx.h"

#include "bsMeshRenderer.h"
#include "bsDx11Renderer.h"
#include "bsVertexTypes.h"
#include "bsTexture2D.h"
#include "bsAssert.h"
#include "bsMesh.h"


bsMeshRenderer::bsMeshRenderer(const bsSharedMesh& mesh,
	const std::shared_ptr<bsTexture2D>& texture,
	const std::shared_ptr<bsPixelShader>& pixelShader,
	const std::shared_ptr<bsVertexShader>& vertexShader)
	: mMesh(mesh)
{
	BS_ASSERT2(mesh, "Mesh cannot be null");
	BS_ASSERT2(texture, "Texture cannot be null");

	mMaterial.diffuse = texture;
	mMaterial.pixelShader = pixelShader;
	mMaterial.vertexShader = vertexShader;
}

void bsMeshRenderer::drawInstanced(ID3D11DeviceContext& deviceContext,
	ID3D11Buffer* instanceBuffer, unsigned int instanceCount) const
{
	if (mMaterial.diffuse != nullptr)
	{
		mMaterial.diffuse->apply(deviceContext, 0);
	}
	if (mMaterial.normal != nullptr)
	{
		mMaterial.normal->apply(deviceContext, 1);
	}

	mMesh->drawInstanced(deviceContext, instanceBuffer, instanceCount);
}

bool bsMeshRenderer::hasFinishedLoading() const
{
	return mMesh->hasFinishedLoading();
}

unsigned int bsMeshRenderer::getTriangleCount() const
{
	return mMesh->getTriangleCount();
}
