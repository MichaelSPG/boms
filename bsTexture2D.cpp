#include "StdAfx.h"

#include "bsTexture2D.h"
#include "bsAssert.h"


bsTexture2D::bsTexture2D(ID3D11ShaderResourceView* texture, ID3D11Device& device,
	unsigned int id)
	: mShaderResourceView(texture)
	, mLoadingCompleted(1)
	, mID(id)
{
	BS_ASSERT2(texture, "Invalid texture");

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	//samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = samplerDesc.BorderColor[1] = samplerDesc.BorderColor[2] =
		samplerDesc.BorderColor[3] = 0.0f;

	const HRESULT hresult = device.CreateSamplerState(&samplerDesc, &mSamplerState);
	BS_ASSERT2(SUCCEEDED(hresult), "Creating sampler state for texture failed");
}

bsTexture2D::~bsTexture2D()
{
	if (mSamplerState != nullptr)
	{
		mSamplerState->Release();
	}
	if (mShaderResourceView != nullptr)
	{
		mShaderResourceView->Release();
	}
}

void bsTexture2D::apply(ID3D11DeviceContext& deviceContext)
{
	if (mLoadingCompleted)
	{
		deviceContext.PSSetShaderResources(0, 1, &mShaderResourceView);
		deviceContext.PSSetSamplers(0, 1, &mSamplerState);
	}
}

void bsTexture2D::loadingCompleted(ID3D11ShaderResourceView* texture, bool success)
{
	if (success)
	{
		mShaderResourceView = texture;

		InterlockedIncrementRelease(&mLoadingCompleted);
	}
}