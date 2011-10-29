#include "StdAfx.h"

#include <string>

#include <Common/Base/hkBase.h>
#include <Common/Visualize/hkDebugDisplay.h>

#include "bsLineRenderer.h"
#include "bsDx11Renderer.h"
#include "bsVertexTypes.h"
#include "bsLog.h"
#include "bsAssert.h"
#include "bsConvert.h"
#include "bsEntity.h"


bsLineRenderer::bsLineRenderer(const XMFLOAT4& colorRgba)
	: mFinished(true)
	, mColor(colorRgba)
	, mVertexBuffer(nullptr)
	, mIndexBuffer(nullptr)
	, mEntity(nullptr)
{
	mBoundingSphere.positionAndRadius = XMVectorSet(0.0f, 0.0f, 0.0f, FLT_MIN);
}

bsLineRenderer::~bsLineRenderer()
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

void bsLineRenderer::addPoint(const XMFLOAT3& position)
{
	mFinished = false;

	mPoints.push_back(position);
}

void bsLineRenderer::addPoints(const XMFLOAT3* points, unsigned int pointCount)
{
	BS_ASSERT(points != nullptr);
	BS_ASSERT2(pointCount != 0, "Cannot add zero points");
	mFinished = false;

	mPoints.insert(mPoints.end(), points, points + pointCount);
}

bool bsLineRenderer::build(bsDx11Renderer* dx11Renderer)
{
	if (mVertexBuffer)
	{
		mVertexBuffer->Release();
	}
	if (mIndexBuffer)
	{
		mIndexBuffer->Release();
	}

	const unsigned int pointCount = mPoints.size();

	BS_ASSERT2(mPoints.size(), "Tried to build line renderer, but no points have been added");
	BS_ASSERT2((mPoints.size() & 1) == 0, "Tried to build line renderer, but an odd amount"
		" of points have been added. Only even numbers of points are supported");

	//Vertex buffer
	D3D11_BUFFER_DESC bufferDescription;
	memset(&bufferDescription, 0, sizeof(bufferDescription));
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(XMFLOAT3) * pointCount;
	bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData;
	memset(&initData, 0, sizeof(initData));
	initData.pSysMem = &mPoints[0];

	if (FAILED(dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, &initData,
		&mVertexBuffer)))
	{
		BS_ASSERT(!"Failed to build vertex buffer");

		return false;
	}

#ifdef BS_DEBUG
	std::string vertexBufferName("VB Line3D");
	mVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, vertexBufferName.length(),
		vertexBufferName.c_str());
#endif // BS_DEBUG

	///Index buffer
	std::vector<unsigned int> indices(pointCount);
	for (unsigned int i = 0; i < pointCount; ++i)
	{
		indices[i] = i;
	}
	
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = pointCount * sizeof(unsigned int);
	bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	initData.pSysMem = &indices[0];

	if (FAILED(dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, &initData,
		&mIndexBuffer)))
	{
		BS_ASSERT(!"Failed to build index buffer");

		return false;
	}
	

#ifdef BS_DEBUG
	std::string indexBufferName("IB Line3D");
	mIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, indexBufferName.length(),
		indexBufferName.c_str());
#endif // BS_DEBUG

	mBoundingSphere = bsCollision::createSphereFromPoints(mPoints.data(), mPoints.size());

	if (mEntity != nullptr)
	{
		mEntity->calculateLocalBoundingSphere();
	}

	mFinished = true;

	return true;
}

void bsLineRenderer::draw(bsDx11Renderer* dx11Renderer) const
{
	//TODO: Don't allow this function to be run when the following is true.
	if (mPoints.empty())
	{
		return;
	}

	BS_ASSERT(dx11Renderer);
	BS_ASSERT2(mFinished, "Trying to draw a bsLineRenderer which has not had its buffers created."
		" Did you forget to call build()?");

	ID3D11DeviceContext* context = dx11Renderer->getDeviceContext();

	const unsigned int offsets =  0;
	const unsigned int stride = sizeof(XMFLOAT3);
	context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offsets);
	context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(mPoints.size(), 0, 0);

	/*
	hkVector4 start, end;

	for (size_t i = 0; i < mPoints.size(); i += 2)
	{
		start = bsMath::toHK(XMLoadFloat3(&mPoints[i]));
		end = bsMath::toHK(XMLoadFloat3(&mPoints[i + 1]));

		HK_DISPLAY_LINE(start, end, 0xffffffff);
	}
	*/
}

void bsLineRenderer::clear()
{
	mPoints.clear();

	if (mVertexBuffer)
	{
		mVertexBuffer->Release();
		mVertexBuffer = nullptr;
	}
	if (mIndexBuffer)
	{
		mIndexBuffer->Release();
		mIndexBuffer = nullptr;
	}
}

void bsLineRenderer::attachedToEntity(bsEntity& entity)
{
	mEntity = &entity;
}
