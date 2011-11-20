#include "StdAfx.h"

#include "bsTexture2D.h"
#include "bsAssert.h"
#include "bsFixedSizeString.h"


bsTexture2D::bsTexture2D(ID3D11ShaderResourceView* texture, ID3D11Device& device,
	unsigned int id, D3D11_FILTER filter /*= D3D11_FILTER_ANISOTROPIC*/)
	: mShaderResourceView(texture)
	, mLoadingCompleted(1)
	, mID(id)
{
	BS_ASSERT2(texture, "Invalid texture");

	D3D11_SAMPLER_DESC samplerDesc;
	//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	//samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.Filter = filter;
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = samplerDesc.BorderColor[1] = samplerDesc.BorderColor[2] =
		samplerDesc.BorderColor[3] = 0.0f;

	HRESULT hresult = device.CreateSamplerState(&samplerDesc, &mSamplerState);
	BS_ASSERT2(SUCCEEDED(hresult), "Creating sampler state for texture failed");

#ifdef BS_DEBUG
	bsString128 debugString = "bsTexture2D sampler state";
	mSamplerState->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());

	unsigned int dataSize = 127;
	hresult = texture->GetPrivateData(WKPDID_D3DDebugObjectName, &dataSize,
		debugString.begin());
	if (hresult == DXGI_ERROR_NOT_FOUND)
	{
		BS_ASSERT2(false, "Texture missing a debug name. Please ensure every texture is"
			" assigned a name when created");
	}
#endif
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

void bsTexture2D::apply(ID3D11DeviceContext& deviceContext, unsigned int slot)
{
	if (mLoadingCompleted)
	{
		deviceContext.PSSetShaderResources(slot, 1, &mShaderResourceView);
		deviceContext.PSSetSamplers(slot, 1, &mSamplerState);
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
