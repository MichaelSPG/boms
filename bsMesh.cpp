#include "bsMesh.h"

#include "bsShaderManager.h"


bsMesh::bsMesh()
	: mID(0)
	, mVertexBuffer(nullptr)
	, mIndexBuffer(nullptr)
	, mIndices(0)
	, mFinished(nullptr)
{
	mAabb.setEmpty();
}

bsMesh::~bsMesh()
{
	for (unsigned int i = 0; i < mSubMeshes.size(); ++i)
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
		//context->DrawIndexedInstanced(mIndices, 5, 0, 0, 0);
	}

	for (unsigned int i = 0; i < mSubMeshes.size(); ++i)
	{
		mSubMeshes[i]->draw(dx11Renderer);
	}
}

void bsMesh::updateAABB()
{
	for (unsigned int i = 0; i < mSubMeshes.size(); ++i)
	{
		mSubMeshes[i]->updateAABB();

		const hkAabb& subMeshAabb = mSubMeshes[i]->mAabb;

		if (!mAabb.contains(subMeshAabb))
		{
			mAabb.includeAabb(subMeshAabb);
		}
	}
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
}
