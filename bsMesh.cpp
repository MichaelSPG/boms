#include "bsMesh.h"

#include <algorithm>

#include <Windows.h>
#include <xnamath.h>

#include "bsShaderManager.h"
#include "bsConstantBuffers.h"
#include "bsVertexTypes.h"
#include "bsDx11Renderer.h"
#include "bsAssert.h"


bsMesh::bsMesh(unsigned int id, std::vector<ID3D11Buffer*>&& vertexBuffers,
	std::vector<ID3D11Buffer*>&& indexBuffers,
	std::vector<unsigned int>&& indices)
	: mVertexBuffers(std::move(vertexBuffers))
	, mIndexBuffers(std::move(indexBuffers))
	, mIndexCounts(std::move(indices))
	, mID(id)
	, mLoadingFinished(true)
{
	BS_ASSERT2(mVertexBuffers.size() == mIndexBuffers.size()
		&& mVertexBuffers.size() == mIndexCounts.size(),
		"A mesh is required to have the same amount of the same amount of vertex and"
		"index buffers, as well as one set of indices for each vertex/index buffer pair");

	BS_ASSERT2(!mVertexBuffers.empty(), "Encountered a mesh with no index or vertex buffers"
		", which most likely means an error has occured with exporting");

	BS_ASSERT2(mVertexBuffers.size() <= D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT,
		"Vertex/index buffer count too large for a single mesh");
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
	mVertexBuffers = std::move(other.mVertexBuffers);
	mIndexBuffers = std::move(other.mIndexBuffers);
	mIndexCounts = std::move(other.mIndexCounts);
	mID = other.mID;

	other.mVertexBuffers.clear();
	other.mIndexBuffers.clear();

	//Set finished flag last so that when it is set, all other data has been set, making
	//it safe to read that data without a mutex/lock.
	//TODO: Verify that this is not stupid.
	mLoadingFinished = other.mLoadingFinished;

	return *this;
}

void bsMesh::draw(bsDx11Renderer* dx11Renderer) const
{
	BS_ASSERT2(mLoadingFinished, "Trying to draw a mesh that has not finished loading yet!");

	ID3D11DeviceContext* context = dx11Renderer->getDeviceContext();

	//Not using any buffers where offset starts at anything but zero.
	const unsigned int offsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { 0 };

	unsigned int strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	for (unsigned int i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i)
	{
		strides[i] = sizeof(VertexNormalTex);
	}

	const size_t bufferCount = mVertexBuffers.size();

	//Draw all vertex/index buffers.
	for (size_t i = 0; i < bufferCount; ++i)
	{
		context->IASetVertexBuffers(0, 1, &mVertexBuffers[i], strides, offsets);
		context->IASetIndexBuffer(mIndexBuffers[i], DXGI_FORMAT_R32_UINT, 0);

		context->DrawIndexed(mIndexCounts[i], 0, 0);
	}
}

void bsMesh::setAabb(const hkVector4& min, const hkVector4& max)
{
	mAabb.m_min = min;
	mAabb.m_max = max;
}
