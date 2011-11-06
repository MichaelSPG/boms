#include "StdAfx.h"

#include "bsRenderTarget.h"

#include <string>

#include "bsDx11Renderer.h"
#include "bsAssert.h"



bsRenderTarget::bsRenderTarget(unsigned int width, unsigned int height,
	ID3D11Device& device)
	: mFormat(DXGI_FORMAT_R16G16B16A16_FLOAT)
	, mRenderTargetTexture(nullptr)
	, mRenderTargetView(nullptr)
	, mShaderResourceView(nullptr)
{
	//DXGI_FORMAT_R32G32B32A32_FLOAT

	//Create the texture
	bool success = createTexture(width, height, device);

	//Create the render target view
	success |= createView(device);

	//Create shader resource
	success |= createShaderResourceView(device);

	//TODO: Do something sensible when something fails.
	(void)success;
}

bsRenderTarget::~bsRenderTarget()
{
	mRenderTargetTexture->Release();
	mRenderTargetView->Release();
	mShaderResourceView->Release();
}

bool bsRenderTarget::windowResized(unsigned int width, unsigned int height,
	ID3D11Device& device)
{
	//Release resources and recreate them.

	mRenderTargetTexture->Release();
	mRenderTargetView->Release();
	mShaderResourceView->Release();


	//Create the texture
	bool success = createTexture(width, height, device);

	//Create the render target view
	success |= createView(device);

	//Create shader resource
	success |= createShaderResourceView(device);

	return success;
}

bool bsRenderTarget::createTexture(unsigned int width, unsigned int height,
	ID3D11Device& device)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	memset(&textureDesc, 0, sizeof(textureDesc));

	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = mFormat;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	const HRESULT hr = device.CreateTexture2D(&textureDesc, nullptr,
		&mRenderTargetTexture);

	BS_ASSERT2(SUCCEEDED(hr), "Failed to create texture");

	return SUCCEEDED(hr);
}

bool bsRenderTarget::createView(ID3D11Device& device)
{
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetDesc;
	memset(&renderTargetDesc, 0, sizeof(renderTargetDesc));

	renderTargetDesc.Format = mFormat;
	renderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetDesc.Texture2D.MipSlice = 0;

	const HRESULT hr = device.CreateRenderTargetView(mRenderTargetTexture, &renderTargetDesc,
		&mRenderTargetView);

	BS_ASSERT2(SUCCEEDED(hr), "Failed to create render target view");

	return SUCCEEDED(hr);
}

bool bsRenderTarget::createShaderResourceView(ID3D11Device& device)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceDesc;
	memset(&shaderResourceDesc, 0, sizeof(shaderResourceDesc));

	shaderResourceDesc.Format = mFormat;
	shaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceDesc.Texture2D.MipLevels = 1;

	const HRESULT hr = device.CreateShaderResourceView(mRenderTargetTexture,
		&shaderResourceDesc, &mShaderResourceView);

	BS_ASSERT2(SUCCEEDED(hr), "Failed to create shader resource view");

	return SUCCEEDED(hr);
}
