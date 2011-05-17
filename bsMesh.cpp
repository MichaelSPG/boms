#include "bsMesh.h"

#include <algorithm>

#include <Windows.h>
#include <xnamath.h>

#include "bsShaderManager.h"
#include "bsConstantBuffers.h"
#include "bsVertexTypes.h"
#include "bsDx11Renderer.h"


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
	if (mVertexBuffer)
	{
		mVertexBuffer->Release();
	}
	if (mIndexBuffer)
	{
		mIndexBuffer->Release();
	}
	std::for_each(mSubMeshes.begin(), mSubMeshes.end(),
		[](bsMesh* mesh)
	{
		delete mesh;	
	});
}

void bsMesh::draw(bsDx11Renderer* dx11Renderer) const
{
	//If both of these are null, this is probably a mesh which only owns sub-meshes
	//without having any defined triangles itself.
	if (mVertexBuffer && mIndices)
	{
		ID3D11DeviceContext* context = dx11Renderer->getDeviceContext();

		UINT offsets =  0;
		UINT stride = sizeof(VertexNormalTex);
		context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offsets);
		context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		context->DrawIndexed(mIndices, 0, 0);
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
