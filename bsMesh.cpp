#include "StdAfx.h"

#include "bsMesh.h"

#include <algorithm>

#include <Windows.h>
#include <xnamath.h>

#include "bsShaderManager.h"
#include "bsConstantBuffers.h"
#include "bsVertexTypes.h"
#include "bsDx11Renderer.h"
#include "bsAssert.h"
#include "bsEntity.h"


bsMesh::bsMesh(unsigned int id, std::vector<ID3D11Buffer*>&& vertexBuffers,
	std::vector<ID3D11Buffer*>&& indexBuffers,
	std::vector<unsigned int>&& indexCounts, std::vector<unsigned int>&& vertexCounts,
	const bsCollision::Sphere& boundingSphere)
	: mBoundingSphere(boundingSphere)
	, mVertexBuffers(std::move(vertexBuffers))
	, mIndexBuffers(std::move(indexBuffers))
	, mIndexCounts(std::move(indexCounts))
	, mVertexCounts(std::move(vertexCounts))
	, mID(id)
	, mLoadingFinished(true)
{
	BS_ASSERT2(mVertexBuffers.size() == mIndexBuffers.size()
		&& mVertexBuffers.size() == mIndexCounts.size(),
		"A mesh is required to have the same amount of the same amount of vertex and"
		"index buffers, as well as one set of indices for each vertex/index buffer pair");

	BS_ASSERT2(!mVertexBuffers.empty(), "Encountered a mesh with no index or vertex buffers"
		", which most likely means an error has occured with exporting");

	BS_ASSERT2(mBoundingSphere.getRadius() > 0.0f, "Invalid bounding sphere");
}

bsMesh::~bsMesh()
{
	const size_t bufferCount = mVertexBuffers.size();

	for (size_t i = 0; i < bufferCount; ++i)
	{
		mVertexBuffers[i]->Release();
	}
	for (size_t i = 0; i < bufferCount; ++i)
	{
		mIndexBuffers[i]->Release();
	}
}

bsMesh& bsMesh::operator=(bsMesh&& other)
{
	mBoundingSphere = other.mBoundingSphere;

	mVertexBuffers = std::move(other.mVertexBuffers);
	mIndexBuffers = std::move(other.mIndexBuffers);
	mIndexCounts = std::move(other.mIndexCounts);
	mVertexCounts = std::move(other.mVertexCounts);
	//mID = other.mID;

	other.mVertexBuffers.clear();
	other.mIndexBuffers.clear();

	//Set finished flag last so that when it is set, all other data has been set, making
	//it safe to read that data without a mutex/lock.
	//TODO: Verify that this is not stupid.
	//mLoadingFinished = other.mLoadingFinished;
	if (other.mLoadingFinished)
	{
		InterlockedIncrementRelease(&mLoadingFinished);
	}

	for (unsigned int i = 0; i < mEntities.size(); ++i)
	{
		mEntities[i]->calculateLocalBoundingSphere();
	}

	return *this;
}

void bsMesh::drawInstanced(ID3D11DeviceContext& deviceContext, ID3D11Buffer* instanceBuffer,
	unsigned int instanceCount) const
{
	if (!mLoadingFinished)
	{
		return;
	}

	unsigned int strides[2] = { sizeof(bsVertexNormalTangentTex), sizeof(XMMATRIX) };
	unsigned int offsets[2] = { 0, 0 };
	ID3D11Buffer* vertexInstanceBuffers[2] = { nullptr, instanceBuffer };

	const unsigned int bufferCount = mVertexBuffers.size();
	for (unsigned int i = 0; i < bufferCount; ++i)
	{
		vertexInstanceBuffers[0] = mVertexBuffers[i];
		deviceContext.IASetVertexBuffers(0, 2, vertexInstanceBuffers, strides, offsets);
		deviceContext.IASetIndexBuffer(mIndexBuffers[i], DXGI_FORMAT_R32_UINT, 0);

		deviceContext.DrawIndexedInstanced(mIndexCounts[i], instanceCount, 0, 0, 0);
	}
}

void bsMesh::attachedToEntity(bsEntity& entity)
{
	mEntities.push_back(&entity);
}
