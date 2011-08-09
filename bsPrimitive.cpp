#include "bsPrimitive.h"

#include <vector>

#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>

#include "bsShaderManager.h"
#include "bsPixelShader.h"
#include "bsVertexShader.h"
#include "bsLog.h"
#include "bsVertexTypes.h"
#include "bsConstantBuffers.h"
#include "bsDx11Renderer.h"
#include "bsAssert.h"


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

void bsPrimitive::createPrimitive(bsDx11Renderer* dx11Renderer,
	bsShaderManager* shaderManager, const hkAabb& aabb)
{
	BS_ASSERT(dx11Renderer);
	BS_ASSERT(shaderManager);

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
	
	mVertexShader = shaderManager->getVertexShader("Wireframe.fx", inputLayout);
	mPixelShader  = shaderManager->getPixelShader("Wireframe.fx");


	D3D11_BUFFER_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(bufferDescription));
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(CBWireFrame);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	HRESULT hres = dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, nullptr,
		&mBuffer);
	BS_ASSERT2(SUCCEEDED(hres), "Failed to create a buffer");

#ifdef BS_DEBUG
	std::string bufferName("CB Primitive");
	mBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, bufferName.length(),
		bufferName.c_str());
#endif // BS_DEBUG

	
	hkVector4 halfExtents;
	aabb.getHalfExtents(halfExtents);
	const hkVector4& maxHalfExtents = aabb.m_max;
	const hkVector4& minHalfExtents = aabb.m_min;
	
	std::vector<XMFLOAT3> vertices(8);
	vertices.push_back(XMFLOAT3(minHalfExtents.getSimdAt(0), maxHalfExtents.getSimdAt(1),
		minHalfExtents.getSimdAt(2)));//Upper left, front
	vertices.push_back(XMFLOAT3(maxHalfExtents.getSimdAt(0), maxHalfExtents.getSimdAt(1),
		minHalfExtents.getSimdAt(2)));//Upper right, front
	vertices.push_back(XMFLOAT3(maxHalfExtents.getSimdAt(0), minHalfExtents.getSimdAt(1),
		minHalfExtents.getSimdAt(2)));//Lower right, front
	vertices.push_back(XMFLOAT3(minHalfExtents.getSimdAt(0), minHalfExtents.getSimdAt(1),
		minHalfExtents.getSimdAt(2)));//Lower left, front

	vertices.push_back(XMFLOAT3(minHalfExtents.getSimdAt(0), maxHalfExtents.getSimdAt(1),
		maxHalfExtents.getSimdAt(2)));//Upper left, back
	vertices.push_back(XMFLOAT3(maxHalfExtents.getSimdAt(0), maxHalfExtents.getSimdAt(1),
		maxHalfExtents.getSimdAt(2)));//Upper right, back
	vertices.push_back(XMFLOAT3(maxHalfExtents.getSimdAt(0), minHalfExtents.getSimdAt(1),
		maxHalfExtents.getSimdAt(2)));//Lower right, back
	vertices.push_back(XMFLOAT3(minHalfExtents.getSimdAt(0), minHalfExtents.getSimdAt(1),
		maxHalfExtents.getSimdAt(2)));//Lower left, back

	
	//Vertex buffer
	bufferDescription.ByteWidth = sizeof(XMFLOAT3) * vertices.size();
	bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = &vertices[0];

	if (FAILED(dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, &initData,
		&mVertexBuffer)))
	{
		BS_ASSERT(!"Failed to create vertex buffer");
	}

#ifdef BS_DEBUG
	std::string vertexBufferName("VB Primitive");
	mVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, vertexBufferName.length(),
		vertexBufferName.c_str());
#endif // BS_DEBUG


	///Index buffer
	unsigned short indices[] =
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
	bufferDescription.ByteWidth = sizeof(unsigned short) * ARRAYSIZE(indices);
	bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	initData.pSysMem = indices;

	hres = dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, &initData,
		&mIndexBuffer);
	BS_ASSERT2(SUCCEEDED(hres), "Failed to create index buffer");

#ifdef BS_DEBUG
	std::string indexBufferName("PrimitiveIndexBuffer");
	mIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, indexBufferName.length(),
		indexBufferName.c_str());
#endif // BS_DEBUG

	mFinished = true;
}

void bsPrimitive::draw(bsDx11Renderer* dx11Renderer)
{
	BS_ASSERT(dx11Renderer);
	BS_ASSERT(mFinished);

	ID3D11DeviceContext* const context = dx11Renderer->getDeviceContext();

	UINT offsets =  0;
	UINT stride = sizeof(XMFLOAT3);
	context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offsets);
	context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	context->IASetInputLayout(mVertexShader->getInputLayout());

	//TODO: Remove the shaders from this class.
	/*
	context->VSSetShader(mVertexShader->getVertexShader(), nullptr, 0);
	context->PSSetShader(mPixelShader->getPixelShader(), nullptr, 0);

	context->VSSetConstantBuffers(1, 1, &mBuffer);
	context->PSSetConstantBuffers(1, 1, &mBuffer);
	*/

	context->DrawIndexed(24, 0, 0);
}
