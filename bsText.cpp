#include "bsText2D.h"

#include <assert.h>
#include <sstream>
#include "bsLog.h"
#include "bsTimer.h"

#include "bsDx11Renderer.h"


bsText2D::bsText2D(bsDx11Renderer* dx11Renderer, const std::wstring& text)
	: mEnabled(true)
	, mText(text)
	, mFontWrapper(nullptr)
	, mDeviceContext(dx11Renderer->getDeviceContext())

	, mColorAbgr(0xFFFFFFFF)
	, mFontSize(12.0f)
	, mPosition(0.0f, 0.0f)
	, mFlags(0)
{
}

bsText2D::~bsText2D()
{
	if (mFontWrapper)
	{
		mFontWrapper->Release();
	}
}

void bsText2D::draw()
{
	if (!mEnabled)
	{
		return;
	}
	
	mFontWrapper->DrawString(mDeviceContext, mText.c_str(), mFontSize,
		mPosition.x, mPosition.y, mColorAbgr, mFlags);
}
