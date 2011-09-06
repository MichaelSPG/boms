#include "StdAfx.h"

#include "bsRenderTarget.h"

#include <string>

#include "bsDx11Renderer.h"
#include "bsAssert.h"



bsRenderTarget::bsRenderTarget(unsigned int width, unsigned int height,
	ID3D11Device* d3d11Device)
	: mRenderTargetTexture(nullptr)
	, mRenderTargetView(nullptr)
	, mShaderResourceView(nullptr)
{
	//Create the texture
	D3D11_TEXTURE2D_DESC textureDesc;
	memset(&textureDesc, 0, sizeof(textureDesc));

	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = d3d11Device->CreateTexture2D(&textureDesc, nullptr,
		&mRenderTargetTexture);
	BS_ASSERT2(SUCCEEDED(hr), "Failed to create texture");

	//Create the render target view
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetDesc;
	memset(&renderTargetDesc, 0, sizeof(renderTargetDesc));

	renderTargetDesc.Format = textureDesc.Format;
	renderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetDesc.Texture2D.MipSlice = 0;

	hr = d3d11Device->CreateRenderTargetView(mRenderTargetTexture, &renderTargetDesc,
		&mRenderTargetView);
	BS_ASSERT2(SUCCEEDED(hr), "Failed to create render target view");

	//Create shader resource
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceDesc;
	memset(&shaderResourceDesc, 0, sizeof(shaderResourceDesc));

	shaderResourceDesc.Format = textureDesc.Format;
	shaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceDesc.Texture2D.MipLevels = 1;

	hr = d3d11Device->CreateShaderResourceView(mRenderTargetTexture, &shaderResourceDesc,
		&mShaderResourceView);
	BS_ASSERT2(SUCCEEDED(hr), "Failed to create shader resource view");

#ifdef BS_DEBUG
	std::string debugString("ID3D11Texture2D render target");
	mRenderTargetTexture->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());

	debugString = "ID3D11RenderTargetView";
	mRenderTargetView->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());

	debugString = "ID3D11ShaderResourceView";
	mShaderResourceView->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());
#endif
}

bsRenderTarget::~bsRenderTarget()
{
	mRenderTargetTexture->Release();
	mRenderTargetView->Release();
	mShaderResourceView->Release();
}
