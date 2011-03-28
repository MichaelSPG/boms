#include "bsText.h"

#include <assert.h>
#include <sstream>
#include "Log.h"
#include "Timer.h"

#include "Dx11Renderer.h"


bsText::bsText(Dx11Renderer* dx11Renderer, const std::wstring& text)
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

bsText::~bsText()
{
	if (mFontWrapper)
	{
		mFontWrapper->Release();
	}
}

void bsText::draw()
{
	if (!mEnabled)
	{
		return;
	}
	
	mFontWrapper->DrawString(mDeviceContext, mText.c_str(), mFontSize,
		mPosition.x, mPosition.y, mColorAbgr, mFlags);
}
