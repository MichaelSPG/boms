#include "Primitive.h"

#include "AABB.h"
#include "ShaderManager.h"
#include "Node.h"
#include "PixelShader.h"
#include "VertexShader.h"


Primitive::Primitive()
{

}

Primitive::~Primitive()
{
	mBuffer->Release();
	mIndexBuffer->Release();
	mVertexBuffer->Release();
}

#define GREEN XMFLOAT3(0.0f, 1.0f, 0.0f)

void Primitive::createPrimitive(Dx11Renderer* dx11Renderer, ShaderManager* shaderManager,
	const AABB& aabb)
{
	assert(dx11Renderer);
	assert(shaderManager);

	mVertexShader = shaderManager->getVertexShader("Wireframe.fx");
	mPixelShader  = shaderManager->getPixelShader("Wireframe.fx");


	D3D11_BUFFER_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(bufferDescription));
	
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(CBChangesEveryFrame);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	HRESULT res = dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, nullptr,
		&mBuffer);
	assert(SUCCEEDED(res));



	const XMFLOAT3& halfExtents = aabb.getHalfExtents();

	//Vertex buffer
	Vertex vertices[] =
	{
		{ XMFLOAT3(-halfExtents.x,  halfExtents.y, -halfExtents.z) },//Upper left, front
		{ XMFLOAT3( halfExtents.x,  halfExtents.y, -halfExtents.z) },//Upper right, front
		{ XMFLOAT3( halfExtents.x, -halfExtents.y, -halfExtents.z) },//Lower right, front
		{ XMFLOAT3(-halfExtents.x, -halfExtents.y, -halfExtents.z) },//Lower left, front

		{ XMFLOAT3(-halfExtents.x,  halfExtents.y, halfExtents.z) },//Upper left, back
		{ XMFLOAT3( halfExtents.x,  halfExtents.y, halfExtents.z) },//Upper right, back
		{ XMFLOAT3( halfExtents.x, -halfExtents.y, halfExtents.z) },//Lower right, back
		{ XMFLOAT3(-halfExtents.x, -halfExtents.y, halfExtents.z) },//Lower left, back
	};

	/*
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3( -2.0f, 2.0f, -2.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 2.0f, 2.0f, -2.0f ), XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( 2.0f, 2.0f, 2.0f ), XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -2.0f, 2.0f, 2.0f ), XMFLOAT2( 0.0f, 1.0f ) },

		{ XMFLOAT3( -2.0f, -2.0f, -2.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 2.0f, -2.0f, -2.0f ), XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( 2.0f, -2.0f, 2.0f ), XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -2.0f, -2.0f, 2.0f ), XMFLOAT2( 0.0f, 1.0f ) },

		{ XMFLOAT3( -2.0f, -2.0f, 2.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( -2.0f, -2.0f, -2.0f ), XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( -2.0f, 2.0f, -2.0f ), XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -2.0f, 2.0f, 2.0f ), XMFLOAT2( 0.0f, 1.0f ) },

		{ XMFLOAT3( 2.0f, -2.0f, 2.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 2.0f, -2.0f, -2.0f ), XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( 2.0f, 2.0f, -2.0f ), XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( 2.0f, 2.0f, 2.0f ), XMFLOAT2( 0.0f, 1.0f ) },

		{ XMFLOAT3( -2.0f, -2.0f, -2.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 2.0f, -2.0f, -2.0f ), XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( 2.0f, 2.0f, -2.0f ), XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -2.0f, 2.0f, -2.0f ), XMFLOAT2( 0.0f, 1.0f ) },

		{ XMFLOAT3( -2.0f, -2.0f, 2.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 2.0f, -2.0f, 2.0f ), XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( 2.0f, 2.0f, 2.0f ), XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -2.0f, 2.0f, 2.0f ), XMFLOAT2( 0.0f, 1.0f ) },
	};
	*/
	//Verte buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertices;

	if (!SUCCEEDED(dx11Renderer->getDevice()->CreateBuffer(&bufferDesc, &initData,
		&mVertexBuffer)))
	{
		throw std::exception("Failed to create a vertex buffer");
	}

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
		3, 4,
		4, 5,
		5, 6,
		6, 7
	};
	/*
	WORD indices[] =
	{
		3,1,0,
		2,1,3,

		6,4,5,
		7,4,6,

		11,9,8,
		10,9,11,

		14,12,13,
		15,12,14,

		19,17,16,
		18,17,19,

		22,20,21,
		23,20,22
	};
	*/
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
	bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	initData.pSysMem = indices;

	res = dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, &initData,
		&mIndexBuffer);
	assert(SUCCEEDED(res));

	const XMFLOAT3& position = aabb.getOwner()->getPosition();
	XMStoreFloat4x4(&mTranslation, XMMatrixTranslation(position.x, position.y, position.z));
}

void Primitive::draw(Dx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);


	UINT offsets = 0;
	UINT stride = sizeof(SimpleVertex);
	dx11Renderer->getDeviceContext()->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride,
		&offsets);
	dx11Renderer->getDeviceContext()->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	//mDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offsets);
	
	CBWireFrame constBuffer;
	XMStoreFloat4x4(&constBuffer.world, XMMatrixTranslation(0.0f, 1000.0f, 10.0f));
	//XMStoreFloat4x4(&constBuffer.mWorld, XMLoadFloat4x4(&mTranslation));
	constBuffer.color = XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f);
	dx11Renderer->getDeviceContext()->UpdateSubresource(mBuffer, 0, nullptr, &constBuffer,
		0, 0);
	dx11Renderer->getDeviceContext()->VSSetConstantBuffers(2, 1, &mBuffer);
	

	

	//dx11Renderer->getDeviceContext()->VSSetShader(mVertexShader, nullptr, 0);

	dx11Renderer->getDeviceContext()->PSSetShader(mPixelShader->getPixelShader(), nullptr, 0);
	dx11Renderer->getDeviceContext()->VSSetShader(mVertexShader->getVertexShader(), nullptr, 0);
	dx11Renderer->getDeviceContext()->IASetInputLayout(mVertexShader->getInputLayout());
	
	dx11Renderer->getDeviceContext()->DrawIndexed(14, 0, 0);
}
