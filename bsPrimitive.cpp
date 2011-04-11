#include "bsPrimitive.h"

#include <assert.h>
#include <vector>

#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>

#include "bsShaderManager.h"
#include "bsPixelShader.h"
#include "bsVertexShader.h"
#include "bsLog.h"
#include "bsVertexTypes.h"


bsPrimitive::bsPrimitive()
	: mBuffer(nullptr)
	, mIndexBuffer(nullptr)
	, mVertexBuffer(nullptr)
	, mColor(0.0f, 1.0f, 0.0f, 0.0f)
	, mFinished(false)
{

}

bsPrimitive::~bsPrimitive()
{
	if (mBuffer)
	{
		mBuffer->Release();
	}
	if (mIndexBuffer)
	{
		mIndexBuffer->Release();
	}
	if (mVertexBuffer)
	{
		mVertexBuffer->Release();
	}
}

void bsPrimitive::createPrimitive(bsDx11Renderer* dx11Renderer, bsShaderManager* shaderManager,
	const hkAabb& aabb, bool nodeAabb /*= true*/)
{
	assert(dx11Renderer);
	assert(shaderManager);

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout;
	D3D11_INPUT_ELEMENT_DESC d;
	d.SemanticName = "POSITION";
	d.SemanticIndex = 0;
	d.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	d.InputSlot = 0;
	d.AlignedByteOffset = 0;
	d.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	d.InstanceDataStepRate = 0;
	inputLayout.push_back(d);
	/*
	d.SemanticName = "COLOR";
	d.AlignedByteOffset = 12;
	inputLayout.push_back(d);
	*/
	mVertexShader = shaderManager->getVertexShader("Wireframe.fx", inputLayout);
	mPixelShader  = shaderManager->getPixelShader("Wireframe.fx");


	D3D11_BUFFER_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(bufferDescription));

	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(CBWireFrame);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = 0;
	bufferDescription.MiscFlags = 0;

	HRESULT hres = dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, nullptr,
		&mBuffer);
	assert(SUCCEEDED(hres));

#ifdef _DEBUG
	const char bufferName[] = "PrimitiveConstantBuffer";
	mBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(bufferName) - 1, bufferName);
#endif // _DEBUG

	
	hkVector4 halfExtents;
	aabb.getHalfExtents(halfExtents);
	hkQuadReal maxQuad(aabb.m_max.getQuad());
	hkQuadReal minQuad(aabb.m_min.getQuad());
	
	std::vector<XMFLOAT3> vertices;
	if (nodeAabb)
	{
		//Vertex buffer
		XMFLOAT3 verticesAabb[] =
		{
			XMFLOAT3(-halfExtents.getSimdAt(0),  halfExtents.getSimdAt(1), -halfExtents.getSimdAt(2)),//Upper left, front
			XMFLOAT3( halfExtents.getSimdAt(0),  halfExtents.getSimdAt(1), -halfExtents.getSimdAt(2)),//Upper right, front
			XMFLOAT3( halfExtents.getSimdAt(0), -halfExtents.getSimdAt(1), -halfExtents.getSimdAt(2)),//Lower right, front
			XMFLOAT3(-halfExtents.getSimdAt(0), -halfExtents.getSimdAt(1), -halfExtents.getSimdAt(2)),//Lower left, front

			XMFLOAT3(-halfExtents.getSimdAt(0),  halfExtents.getSimdAt(1), halfExtents.getSimdAt(2)),//Upper left, back
			XMFLOAT3( halfExtents.getSimdAt(0),  halfExtents.getSimdAt(1), halfExtents.getSimdAt(2)),//Upper right, back
			XMFLOAT3( halfExtents.getSimdAt(0), -halfExtents.getSimdAt(1), halfExtents.getSimdAt(2)),//Lower right, back
			XMFLOAT3(-halfExtents.getSimdAt(0), -halfExtents.getSimdAt(1), halfExtents.getSimdAt(2)),//Lower left, back
		};

		for (unsigned int i = 0; i < ARRAYSIZE(verticesAabb); ++i)
		{
			vertices.push_back(verticesAabb[i]);
		}
	}
	else
	{
		//Vertex buffer
		XMFLOAT3 verticesNotAabb[] =
		{
			XMFLOAT3(minQuad.x, maxQuad.y, minQuad.z),//Upper left, front
			XMFLOAT3(maxQuad.x, maxQuad.y, minQuad.z),//Upper right, front
			XMFLOAT3(maxQuad.x, minQuad.y, minQuad.z),//Lower right, front
			XMFLOAT3(minQuad.x, minQuad.y, minQuad.z),//Lower left, front

			XMFLOAT3(minQuad.x, maxQuad.y, maxQuad.z),//Upper left, back
			XMFLOAT3(maxQuad.x, maxQuad.y, maxQuad.z),//Upper right, back
			XMFLOAT3(maxQuad.x, minQuad.y, maxQuad.z),//Lower right, back
			XMFLOAT3(minQuad.x, minQuad.y, maxQuad.z),//Lower left, back
		};

		for (unsigned int i = 0; i < ARRAYSIZE(verticesNotAabb); ++i)
		{
			vertices.push_back(verticesNotAabb[i]);
		}
	}

	
	
	
	//Vertex buffer
	bufferDescription.ByteWidth = sizeof(XMFLOAT3) * vertices.size();
	bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = &vertices[0];

	if (FAILED(dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, &initData,
		&mVertexBuffer)))
	{
		bsLog::logMessage("bsPrimitive failed to create vertex buffer");

		assert(!"bsPrimitive failed to create vertex buffer");
	}

#ifdef _DEBUG
	const char vertexBufferName[] = "PrimitiveVertexBuffer";
	mVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(vertexBufferName) - 1,
		vertexBufferName);
#endif // _DEBUG

	/*
	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;
	dx11Renderer->getDeviceContext()->IASetVertexBuffers(1, 1, &mVertexBuffer, &stride,
		&offset);
	*/

	///Index buffer
	WORD indices[] =
	{
		0, 1,
		1, 2,
		2, 3,
		3, 0,
		4, 5,
		5, 6,
		6, 7,
		7, 4,
		4, 0,
		1, 5,
		6, 2,
		7, 3
	};
	
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
	bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	initData.pSysMem = indices;

	hres = dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, &initData,
		&mIndexBuffer);
	assert(SUCCEEDED(hres));

#ifdef _DEBUG
	const char indexBufferName[] = "PrimitiveIndexBuffer";
	mIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(indexBufferName) - 1,
		indexBufferName);
#endif // _DEBUG

	mFinished = true;
}

void bsPrimitive::draw(bsDx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);
	assert(mFinished);

	auto context = dx11Renderer->getDeviceContext();


	UINT offsets =  0;
	UINT stride = sizeof(XMFLOAT3);
	context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offsets);
	context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	context->IASetInputLayout(mVertexShader->getInputLayout());

	context->VSSetShader(mVertexShader->getVertexShader(), nullptr, 0);
	context->PSSetShader(mPixelShader->getPixelShader(), nullptr, 0);

	context->VSSetConstantBuffers(1, 1, &mBuffer);
	context->PSSetConstantBuffers(1, 1, &mBuffer);


	context->DrawIndexed(24, 0, 0);
}
