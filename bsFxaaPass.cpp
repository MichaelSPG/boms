#include "StdAfx.h"

#include "bsFxaaPass.h"

#include <vector>

#include <d3d11.h>

#include "bsShaderManager.h"
#include "bsConstantBuffers.h"
#include "bsAssert.h"


bsFxaaPass::bsFxaaPass(bsShaderManager* shaderManager, bsDx11Renderer* dx11Renderer,
	float screenWidth, float screenHeight)
	: mDeviceContext(dx11Renderer->getDeviceContext())
	, mShaderManager(shaderManager)
	, mFullscreenQuad(dx11Renderer->getDevice())
	, mFxaaPixelShader(shaderManager->getPixelShader("Fxaa.fx"))
	, mPassthroughPixelShader(shaderManager->getPixelShader("Passthrough.fx"))
	, mOneOverScreenSize(1.0f / screenWidth, 1.0f / screenHeight)
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout(2);
	D3D11_INPUT_ELEMENT_DESC inputDesc;
	memset(&inputDesc, 0, sizeof(inputDesc));
	inputDesc.SemanticName = "POSITION";
	inputDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayout[0] = inputDesc;

	inputDesc.SemanticName = "TEXCOORD";
	inputDesc.AlignedByteOffset = 12;
	inputDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	inputLayout[1] = inputDesc;

	mVertexShader = mShaderManager->getVertexShader("Fxaa.fx", inputLayout.data(), inputLayout.size());


	D3D11_BUFFER_DESC bufferDescription = { 0 };
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(CBFxaa);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	HRESULT hres = dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, nullptr,
		&mOneOverScreenSizeBuffer);

	BS_ASSERT2(SUCCEEDED(hres), "bsRenderQueue::bsRenderQueue failed to create world buffer");
}

bsFxaaPass::~bsFxaaPass()
{
	mOneOverScreenSizeBuffer->Release();
}

void bsFxaaPass::draw()
{
	mShaderManager->setVertexShader(mVertexShader);
	mShaderManager->setPixelShader(mFxaaPixelShader);

	CBFxaa constantBuffer;
	constantBuffer.oneOverScreenSize = mOneOverScreenSize;

	mDeviceContext->PSSetConstantBuffers(4, 1, &mOneOverScreenSizeBuffer);
	mDeviceContext->UpdateSubresource(mOneOverScreenSizeBuffer, 0, nullptr,
		&constantBuffer, 0, 0);

	mFullscreenQuad.draw(mDeviceContext);
}
