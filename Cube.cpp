#include "Cube.h"

void Cube::create(Dx11Renderer* renderer, ShaderManager* shaderManager)
{
	mVertexShader = shaderManager->getVertexShader("Wireframe.fx");
	
	mPixelShader = shaderManager->getPixelShader("Wireframe.fx");
	

	VSin vertices[] =
	{
		
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f , 1.0f) },
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f , 1.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 1.0f, 1.0f , 1.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 1.0f , 1.0f ) },

		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f , 1.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f , 1.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 1.0f, 1.0f , 1.0f ) },
		{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, 1.0f , 1.0f ) },

		{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f , 1.0f ) },
		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f , 1.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( 1.0f, 1.0f , 1.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 1.0f , 1.0f ) },

		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f , 1.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f , 1.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 1.0f, 1.0f , 1.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 1.0f , 1.0f ) },

		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f , 1.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f , 1.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 1.0f, 1.0f , 1.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 1.0f , 1.0f ) },

		{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f , 1.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 1.0f, 0.0f , 1.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 1.0f, 1.0f , 1.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 1.0f , 1.0f ) },
	};




	D3D11_BUFFER_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(bufferDescription));

	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(VSin) * ARRAYSIZE(vertices);
	bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));

	initData.pSysMem = vertices;

	Log::log("Creating vertex buffer", LOG_SEV_INFORMATIONAL);
	if (!SUCCEEDED(renderer->getDevice()->CreateBuffer(&bufferDescription, &initData, &mVertexBuffer)))
		throw std::exception("Failed to create vertex buffer");

	//renderer->getDeviceContext()->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);

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


	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
	bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	initData.pSysMem = indices;

	Log::log("Creating index buffer", LOG_SEV_INFORMATIONAL);
	if (!SUCCEEDED(renderer->getDevice()->CreateBuffer(&bufferDescription, &initData, &mIndexBuffer)))
		throw std::exception("Failed to create index buffer.");


	//Constant buffers
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(CBChangesEveryFrame);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	if (!SUCCEEDED(renderer->getDevice()->CreateBuffer(&bufferDescription, nullptr, &mEveryFrame)))
		throw std::exception("Failed to create constant buffer");
}

void Cube::draw(Dx11Renderer* renderer)
{
	auto context = renderer->getDeviceContext();

	UINT offsets =  0;
	UINT stride = sizeof(VSin);
	context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offsets);
	context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	
	context->IASetInputLayout(mVertexShader->getInputLayout());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(mVertexShader->getVertexShader(), nullptr, 0);
	context->PSSetShader(mPixelShader->getPixelShader(), nullptr, 0);
	
	static float t = 0.0f;
	t += 0.01f;
	XMStoreFloat4x4(&mWorld, XMMatrixRotationY(sin(t)));

	

	static bool once = false;
	//if (!once)
	{
		CBChangesEveryFrame cb;
		static float x(120.0f);
		static float y(240.0f);
		static float z(0.0f);

		x += 1.1f;
		y += 1.1f;
		z += 1.1f;


		//cb.vMeshColor = XMFLOAT4(sin(x), sin(y), sin(z), 1.0f);
		cb.vMeshColor = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
		cb.mWorld = mWorld;
		context->UpdateSubresource(mEveryFrame, 0, nullptr, &cb, 0, 0);
		context->VSSetConstantBuffers(2, 1, &mEveryFrame);
		context->PSSetConstantBuffers(2, 1, &mEveryFrame);
		

		once = true;
	}
	

	context->DrawIndexed(36, 0, 0);
}
