#include "StdAfx.h"

#include "bsText3D.h"
#include "bsAssert.h"

#include <D3D11.h>
#include <FW1FontWrapper.h>


bsText3D::bsText3D(ID3D11DeviceContext* deviceContext, ID3D11Device* device, IFW1Factory* fw1Factory)
	: mDeviceContext(deviceContext)
	, mFontWrapper(nullptr)
	, mFont(L"Consolas")
	, mFontSize(20.0f)
	, mColorAabbggrr(0xffffffff)
{
	const HRESULT hres = fw1Factory->CreateFontWrapper(device, mFont.c_str(), &mFontWrapper);
	BS_ASSERT2(SUCCEEDED(hres), "Failed to create a font wrapper object");
}

bsText3D::~bsText3D()
{
	if (mFontWrapper)
	{
		mFontWrapper->Release();
	}
}

void bsText3D::draw(const XMMATRIX& transform)
{
	FW1_RECTF rect = { 0 };

	mFontWrapper->DrawString(mDeviceContext, mText.c_str(), mFont.c_str(), mFontSize, &rect,
		mColorAabbggrr, nullptr, reinterpret_cast<const float*>(&transform),
		FW1_CENTER | FW1_RESTORESTATE | FW1_VCENTER | FW1_NOWORDWRAP);
}
