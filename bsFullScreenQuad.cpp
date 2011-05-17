#include "bsFullScreenQuad.h"

#include <vector>

#include "bsLog.h"
#include "bsVertexTypes.h"
#include "bsAssert.h"


bsFullScreenQuad::bsFullScreenQuad(ID3D11Device* const device)
	: mVertexBuffer(nullptr)
	, mIndexBuffer(nullptr)
{
	BS_ASSERT(device);

	VertexTex vertices[] = 
	{
		XMFLOAT3( 1.0f,  1.0f,  0.0f), XMFLOAT2(1.0f, 0.0f), //Upper right
		XMFLOAT3( 1.0f, -1.0f,  0.0f), XMFLOAT2(1.0f, 1.0f), //Lower right
		XMFLOAT3(-1.0f, -1.0f,  0.0f), XMFLOAT2(0.0f, 1.0f), //Lower left
		XMFLOAT3(-1.0f,  1.0f,  0.0f), XMFLOAT2(0.0f, 0.0f)  //Upper left
	};

	unsigned short indices[] = 
	{
		0, 1, 2,
		2, 3, 0
	};

	//Vertex buffer
	D3D11_BUFFER_DESC bufferDescription;
	memset(&bufferDescription, 0, sizeof(bufferDescription));

	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(VertexTex) * ARRAYSIZE(vertices);
	bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData;
	memset(&initData, 0, sizeof(initData));
	initData.pSysMem = &vertices[0];

	if (FAILED(device->CreateBuffer(&bufferDescription, &initData,
		&mVertexBuffer)))
	{
		bsLog::logMessage("Failed to create vertex buffer for fullscreen quad",
			pantheios::SEV_ERROR);
	}

	//Index buffer
	bufferDescription.ByteWidth = sizeof(unsigned short) * ARRAYSIZE(indices);
	bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	initData.pSysMem = &indices[0];

	if (FAILED(device->CreateBuffer(&bufferDescription, &initData,
		&mIndexBuffer)))
	{
		bsLog::logMessage("Failed to create index buffer for fullscreen quad",
			pantheios::SEV_ERROR);
	}

	//Sampler state
	D3D11_SAMPLER_DESC samplerDesc;
	memset(&samplerDesc, 0, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	
	if (FAILED(device->CreateSamplerState(&samplerDesc, &mSamplerState)))
	{
		bsLog::logMessage("Failed to create sampler state for fullscreen quad",
			pantheios::SEV_ERROR);
	}


#if BS_DEBUG_LEVEL > 0
	std::string debugString("VertexBuffer fullscreen quad");
	
	mVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());

	debugString = "IndexBuffer fullscreen quad";
	mIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());

	debugString = "ID3D11SamplerState fullscreen quad";

	mSamplerState->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());
#endif
}

bsFullScreenQuad::~bsFullScreenQuad()
{
	mVertexBuffer->Release();
	mIndexBuffer->Release();

	mSamplerState->Release();
}

void bsFullScreenQuad::draw(ID3D11DeviceContext* const deviceContext) const
{
	deviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	UINT offsets =  0;
	UINT stride = sizeof(VertexTex);
	deviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offsets);

	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->PSSetSamplers(0, 1, &mSamplerState);

	deviceContext->DrawIndexed(6, 0, 0);
}
