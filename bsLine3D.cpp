#include "bsLine3D.h"

#include <cassert>

#include "bsDx11Renderer.h"
#include "bsVertexTypes.h"
#include "bsLog.h"


bsLine3D::bsLine3D(const XMFLOAT4& color)
	: mFinished(false)
	, mColor(color)
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
	assert(isOkForRendering());

	const unsigned int pointCount = mPoints.size();

	if (!pointCount)
	{
		bsLog::logMessage("bsLine3D::create called with no points defined",
			pantheios::SEV_ERROR);
		assert(!"bsLine3D::create called with no points defined");
		return false;
	}
	else if (pointCount & 1)//if odd
	{
		bsLog::logMessage("Attempted to create bsLine3D with an odd amount of points",
			pantheios::SEV_ERROR);
		assert(!"Attempted to create bsLine3D with an odd amount of points");
		return false;
	}

	//Vertex buffer
	D3D11_BUFFER_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(bufferDescription));
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(XMFLOAT3) * pointCount;
	bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = &mPoints[0];

	if (FAILED(dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, &initData,
		&mVertexBuffer)))
	{
		bsLog::logMessage("bsLine3D failed to create vertex buffer", pantheios::SEV_ERROR);

		assert(!"bsLine3D failed to create vertex buffer");

		return false;
	}

#ifdef _DEBUG
	const char vertexBufferName[] = "Line3D vertex buffer";
	mVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(vertexBufferName) - 1,
		vertexBufferName);
#endif // _DEBUG

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
		bsLog::logMessage("bsLine3D failed to create index buffer", pantheios::SEV_ERROR);
		assert(!"bsLine3D failed to create index buffer");

		return false;
	}
	

#ifdef _DEBUG
	const char indexBufferName[] = "bsLine3D index buffer";
	mIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(indexBufferName) - 1,
		indexBufferName);
#endif // _DEBUG

	mFinished = true;

	return true;
}

void bsLine3D::draw(bsDx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);
	assert(mFinished && "Trying to draw a bsLine3D which has not had its buffers created");

	auto context = dx11Renderer->getDeviceContext();

	UINT offsets =  0;
	UINT stride = sizeof(XMFLOAT3);
	context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offsets);
	context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(mPoints.size(), 0, 0);
	//context->DrawIndexedInstanced(mPoints.size(), 1, 0, 0, 0);
}
