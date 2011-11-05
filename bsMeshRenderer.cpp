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

void bsMeshRenderer::draw(bsDx11Renderer* dx11Renderer) const
{
	/*if (!mMesh->hasFinishedLoading())
	{
		return;
	}

	ID3D11DeviceContext* context = dx11Renderer->getDeviceContext();

	//Not using any buffers where offset starts at anything but zero.
	const unsigned int offsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { 0 };

	unsigned int strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	for (unsigned int i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i)
	{
		strides[i] = sizeof(bsVertexNormalTex);
	}

	
	const std::vector<ID3D11Buffer*>& vertexBuffers = mMesh->getVertexBuffers();
	const std::vector<ID3D11Buffer*>& indexBuffers = mMesh->getIndexBuffers();
	const std::vector<unsigned int>& indexCounts = mMesh->getIndexCounts();

	const size_t bufferCount = vertexBuffers.size();

	//Draw all vertex/index buffers.
	for (size_t i = 0; i < bufferCount; ++i)
	{
		context->IASetVertexBuffers(0, 1, &vertexBuffers[i], strides, offsets);
		context->IASetIndexBuffer(indexBuffers[i], DXGI_FORMAT_R32_UINT, 0);

		context->DrawIndexed(indexCounts[i], 0, 0);
	}*/

	
	if (mMaterial.diffuse != nullptr)
	{
		mMaterial.diffuse->apply(*dx11Renderer->getDeviceContext());
		//mTexture->apply(deviceContext);
	}

	mMesh->draw(dx11Renderer);
}

void bsMeshRenderer::drawInstanced(ID3D11DeviceContext& deviceContext,
	ID3D11Buffer* instanceBuffer, unsigned int instanceCount) const
{
	if (mMaterial.diffuse != nullptr)
	{
		mMaterial.diffuse->apply(deviceContext);
		//mTexture->apply(deviceContext);
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
