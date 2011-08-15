#include "bsMesh.h"

#include <algorithm>

#include <Windows.h>
#include <xnamath.h>

#include "bsShaderManager.h"
#include "bsConstantBuffers.h"
#include "bsVertexTypes.h"
#include "bsDx11Renderer.h"


bsMesh::bsMesh(unsigned int id, unsigned int numSubMeshes, ID3D11Buffer* vertexBuffer,
	ID3D11Buffer* indexBuffer, unsigned int indices)
	: mID(id)
	, mVertexBuffer(vertexBuffer)
	, mIndexBuffer(indexBuffer)
	, mIndices(indices)
	, mFinished(true)
{
	mAabb.setEmpty();
}

bsMesh::bsMesh(bsMesh&& other)
	: mVertexBuffer(other.mVertexBuffer)
	, mIndexBuffer(other.mIndexBuffer)
	, mIndices(other.mIndices)
	, mSubMeshes(std::move(other.mSubMeshes))
	, mID(other.mID)
	, mFinished(other.mFinished)
{
	other.mVertexBuffer = nullptr;
	other.mIndexBuffer = nullptr;
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
}

bsMesh& bsMesh::operator=(bsMesh&& other)
{
	mVertexBuffer = other.mVertexBuffer;
	mIndexBuffer = other.mIndexBuffer;
	mIndices = other.mIndices;
	mSubMeshes = std::move(other.mSubMeshes);
	mID = other.mID;
	mFinished = other.mFinished;

	other.mVertexBuffer = nullptr;
	other.mIndexBuffer = nullptr;
	other.mIndices = 0;

	return *this;
}

void bsMesh::draw(bsDx11Renderer* dx11Renderer) const
{
	//If both of these are null, this is probably a mesh which only owns sub-meshes
	//without having any defined triangles itself.
	if (mVertexBuffer && mIndices)
	{
		ID3D11DeviceContext* context = dx11Renderer->getDeviceContext();

		const UINT offsets =  0;
		const UINT stride = sizeof(VertexNormalTex);
		context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offsets);
		context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		context->DrawIndexed(mIndices, 0, 0);
	}

	for (unsigned int i = 0; i < mSubMeshes.size(); ++i)
	{
		mSubMeshes[i].draw(dx11Renderer);
	}
}

void bsMesh::setAabb(const hkVector4& min, const hkVector4& max)
{
	mAabb.m_min = min;
	mAabb.m_max = max;

	updateAabb();
}

void bsMesh::updateAabb()
{
	//Update all sub meshes' AABBs, and then include them all in this mesh' AABB.
	for (unsigned int i = 0; i < mSubMeshes.size(); ++i)
	{
		mSubMeshes[i].updateAabb();

		const hkAabb& subMeshAabb = mSubMeshes[i].mAabb;

		if (!mAabb.contains(subMeshAabb))
		{
			mAabb.includeAabb(subMeshAabb);
		}
	}
}
