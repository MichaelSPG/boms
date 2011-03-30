#include "bsMesh.h"

#include "bsShaderManager.h"


bsMesh::bsMesh()
	: mID(~0u)
	, mVertexBuffer(nullptr)
	, mIndexBuffer(nullptr)
	, mIndices(0)
	, mAabb(hkVector4(0.0f, 0.0f, 0.0f, 0.0f), hkVector4(0.0f, 0.0f, 0.0f, 0.0f))
	, mDrawableAabbActive(false)
	, mAabbPrimitive(nullptr)
{
}

bsMesh::~bsMesh()
{
	for (unsigned int i = 0u; i < mSubMeshes.size(); ++i)
	{
		delete mSubMeshes[i];
	}
	if (mVertexBuffer)
	{
		mVertexBuffer->Release();
	}
	if (mIndexBuffer)
	{
		mIndexBuffer->Release();
	}
	if (mAabbPrimitive)
	{
		delete mAabbPrimitive;
	}
}

void bsMesh::draw(bsDx11Renderer* dx11Renderer) const
{
	if (mVertexBuffer && mIndices)
	{
		auto context = dx11Renderer->getDeviceContext();

		UINT offsets =  0;
		UINT stride = sizeof(VertexNormalTex);
		context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offsets);
		context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->DrawIndexed(mIndices, 0, 0);
	}

	for (unsigned int i = 0u; i < mSubMeshes.size(); ++i)
	{
		mSubMeshes[i]->draw(dx11Renderer);
	}
	if (mDrawableAabbActive)
	{
	//	mAabbPrimitive->draw(dx11Renderer);
	}
}

void bsMesh::updateAABB()
{
	for (unsigned int i = 0u; i < mSubMeshes.size(); ++i)
	{
		mSubMeshes[i]->updateAABB();

		const hkAabb& subMeshAabb = mSubMeshes[i]->mAabb;

		if (!mAabb.contains(subMeshAabb))
		{
			mAabb.includeAabb(subMeshAabb);
		}
	}
}

void bsMesh::createDrawableAabb(bsDx11Renderer* dx11Renderer, bsShaderManager* shaderManager)
{
	mAabbPrimitive = new bsPrimitive();
	mAabbPrimitive->createPrimitive(dx11Renderer, shaderManager, mAabb, false);

	mDrawableAabbActive = true;
}

bool bsMesh::operator<(const bsMesh& other)
{
	if (other.mID < mID)
	{
		return true;
	}
	//else if (other.mID > mID)
	else
	{
		return false;
	}
//	else
	{
//		return other.mSceneNode < mSceneNode;
	}
}
