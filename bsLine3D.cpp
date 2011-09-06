#include "StdAfx.h"

#include "bsLine3D.h"

#include <string>

#include "bsDx11Renderer.h"
#include "bsVertexTypes.h"
#include "bsLog.h"
#include "bsAssert.h"


bsLine3D::bsLine3D(const XMFLOAT4& colorRgba)
	: mFinished(false)
	, mColor(colorRgba)
	, mVertexBuffer(nullptr)
	, mIndexBuffer(nullptr)
{
}

bsLine3D::~bsLine3D()
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

void bsLine3D::addPoint(const XMFLOAT3& position)
{
	mFinished = false;

	mPoints.push_back(position);
}

void bsLine3D::addPoints(const std::vector<XMFLOAT3>& points)
{
	mFinished = false;

	mPoints.insert(mPoints.end(), points.begin(), points.end());
}

bool bsLine3D::create(bsDx11Renderer* dx11Renderer)
{
	BS_ASSERT2(hasFinishedLoading(), "Tried to create line while the data was in a non-good state");

	const unsigned int pointCount = mPoints.size();

	BS_ASSERT2(pointCount, "No points defined");
	BS_ASSERT2(pointCount & 1, "Attempted to create bsLine3D with an odd amount of points");

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
		BS_ASSERT(!"Failed to create vertex buffer");

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
	bufferDescription.ByteWidth = pointCount * sizeof(indices[0]);
	bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	initData.pSysMem = &indices[0];

	if (FAILED(dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, &initData,
		&mIndexBuffer)))
	{
		BS_ASSERT(!"Failed to create index buffer");

		return false;
	}
	

#ifdef BS_DEBUG
	std::string indexBufferName("IB Line3D");
	mIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, indexBufferName.length(),
		indexBufferName.c_str());
#endif // BS_DEBUG

	mFinished = true;

	return true;
}

void bsLine3D::draw(bsDx11Renderer* dx11Renderer)
{
	BS_ASSERT(dx11Renderer);
	BS_ASSERT2(mFinished, "Trying to draw a bsLine3D which has not had its buffers created");

	ID3D11DeviceContext* context = dx11Renderer->getDeviceContext();

	UINT offsets =  0;
	UINT stride = sizeof(XMFLOAT3);
	context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offsets);
	context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(mPoints.size(), 0, 0);
}
