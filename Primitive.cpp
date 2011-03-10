#include "Primitive.h"

#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>

#include "ShaderManager.h"
#include "Node.h"
#include "PixelShader.h"
#include "VertexShader.h"


Primitive::Primitive()
	: mBuffer(nullptr)
	, mIndexBuffer(nullptr)
	, mVertexBuffer(nullptr)
	, mColor(0.0f, 1.0f, 0.0f, 0.0f)
{

}

Primitive::~Primitive()
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

void Primitive::createPrimitive(Dx11Renderer* dx11Renderer, ShaderManager* shaderManager,
	const hkAabb& aabb)
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

	d.SemanticName = "COLOR";
	d.AlignedByteOffset = 12;
	inputLayout.push_back(d);

	mVertexShader = shaderManager->getVertexShader("../assets/Wireframe.fx", inputLayout);
	mPixelShader  = shaderManager->getPixelShader("../assets/Wireframe.fx");


	D3D11_BUFFER_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(bufferDescription));

	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(CBWireFrame);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = 0;
	bufferDescription.MiscFlags = 0;

	assert(SUCCEEDED(dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, nullptr,
		&mBuffer)));

#ifdef _DEBUG
	const char bufferName[] = "PrimitiveConstantBuffer";
	mBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(bufferName) - 1, bufferName);
#endif // _DEBUG

	



	hkVector4 halfExtents;
	aabb.getHalfExtents(halfExtents);
	//const XMFLOAT3& halfExtents = aabb.getHalfExtents();

	

	//Vertex buffer
	Vertex vertices[] =
	{
		{ XMFLOAT3(-halfExtents.getSimdAt(0),  halfExtents.getSimdAt(1), -halfExtents.getSimdAt(2)) },//Upper left, front
		{ XMFLOAT3( halfExtents.getSimdAt(0),  halfExtents.getSimdAt(1), -halfExtents.getSimdAt(2)) },//Upper right, front
		{ XMFLOAT3( halfExtents.getSimdAt(0), -halfExtents.getSimdAt(1), -halfExtents.getSimdAt(2)) },//Lower right, front
		{ XMFLOAT3(-halfExtents.getSimdAt(0), -halfExtents.getSimdAt(1), -halfExtents.getSimdAt(2)) },//Lower left, front

		{ XMFLOAT3(-halfExtents.getSimdAt(0),  halfExtents.getSimdAt(1), halfExtents.getSimdAt(2)) },//Upper left, back
		{ XMFLOAT3( halfExtents.getSimdAt(0),  halfExtents.getSimdAt(1), halfExtents.getSimdAt(2)) },//Upper right, back
		{ XMFLOAT3( halfExtents.getSimdAt(0), -halfExtents.getSimdAt(1), halfExtents.getSimdAt(2)) },//Lower right, back
		{ XMFLOAT3(-halfExtents.getSimdAt(0), -halfExtents.getSimdAt(1), halfExtents.getSimdAt(2)) },//Lower left, back
	};


	//Vertex buffer
	bufferDescription.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertices;

	if (FAILED(dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, &initData,
		&mVertexBuffer)))
	{
		throw std::exception("Failed to create a vertex buffer");
	}

#ifdef _DEBUG
	const char vertexBufferName[] = "PrimitiveVertexBuffer";
	mVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(vertexBufferName) - 1,
		vertexBufferName);
#endif // _DEBUG

	unsigned int stride = sizeof(SimpleVertex);
	unsigned int offset = 0;
	dx11Renderer->getDeviceContext()->IASetVertexBuffers(1, 1, &mVertexBuffer, &stride,
		&offset);


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

	assert(SUCCEEDED(dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, &initData,
		&mIndexBuffer)));

#ifdef _DEBUG
	const char indexBufferName[] = "PrimitiveIndexBuffer";
	mIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(indexBufferName) - 1,
		indexBufferName);
#endif // _DEBUG
}

void Primitive::draw(Dx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);

	auto context = dx11Renderer->getDeviceContext();


	UINT offsets2 =  0;
	UINT stride2 = sizeof(Vertex);
	context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride2, &offsets2);
	context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	context->IASetInputLayout(mVertexShader->getInputLayout());

	context->VSSetShader(mVertexShader->getVertexShader(), nullptr, 0);
	context->PSSetShader(mPixelShader->getPixelShader(), nullptr, 0);

	context->VSSetConstantBuffers(1, 1, &mBuffer);
	context->PSSetConstantBuffers(1, 1, &mBuffer);


	context->DrawIndexed(24, 0, 0);
}
