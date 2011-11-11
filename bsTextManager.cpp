#include "StdAfx.h"

#include "bsTextManager.h"

#include "bsDx11Renderer.h"
#include "bsLog.h"
#include "bsAssert.h"
#include "bsScrollingText2D.h"
#include "bsFrameStatistics.h"
#include "bsTimer.h"


bsTextManager::bsTextManager(bsDx11Renderer* dx11Renderer)
	: mFw1Factory(nullptr)
	, mDx11Renderer(dx11Renderer)
{
	HRESULT hres = FW1CreateFactory(FW1_VERSION, &mFw1Factory);
	
	BS_ASSERT2(SUCCEEDED(hres), "FW1 font factory creation failed");
}

bsTextManager::~bsTextManager()
{
	mTextBoxes.clear();
	mTexts.clear();
	if (mFw1Factory)
	{
		const unsigned long remainingReferences = mFw1Factory->Release();
		BS_ASSERT2(remainingReferences == 0, "There are remaining references to text"
			" objects. Please release all of them before bsTextManager is destroyed");
	}
}

std::shared_ptr<bsText2D> bsTextManager::createText2D(const std::wstring& text,
	const std::wstring& font /*= L"Consolas"*/)
{
	IFW1FontWrapper* fontWrapper;
	
	HRESULT hres = mFw1Factory->CreateFontWrapper(mDx11Renderer->getDevice(), font.c_str(),
		&fontWrapper);

	BS_ASSERT2(SUCCEEDED(hres), "Failed to create a font wrapper object");
	
	std::shared_ptr<bsText2D> textObject(new bsText2D(mDx11Renderer, text));
	textObject->mFontWrapper = fontWrapper;

	mTexts.push_back(textObject);
	return textObject;
}

std::shared_ptr<bsScrollingText2D> bsTextManager::createScrollingText2D(float fadeDelay,
	unsigned int maxLineCount, const std::wstring& font /*= L"Consolas"*/)
{
	std::shared_ptr<bsScrollingText2D> textBox(new bsScrollingText2D(fadeDelay, maxLineCount));
	textBox->mText = createText2D(L"", font);

	mTextBoxes.push_back(textBox);

	return textBox;
}

void bsTextManager::destroyUnusedTexts()
{
	mTexts.erase(std::remove_if(mTexts.begin(), mTexts.end(),
		[](const std::shared_ptr<bsText2D>& text)
	{
		//If only used by this class, it can be removed
		return text.use_count() == 1;
	}), mTexts.end());

	mTextBoxes.erase(std::remove_if(mTextBoxes.begin(), mTextBoxes.end(),
		[](const std::shared_ptr<bsScrollingText2D>& text)
	{
		return text.use_count() == 1;
	}), mTextBoxes.end());
}

void bsTextManager::drawAllTexts(bsFrameStatistics& frameStatistics)
{
	bsTimer timer;
	float preDraw = timer.getTimeMilliSeconds();

	std::for_each(mTexts.begin(), mTexts.end(),
		[](const std::shared_ptr<bsText2D>& textObject)
	{
		textObject->draw();
	});

	frameStatistics.renderingInfo.textDuration = timer.getTimeMilliSeconds() - preDraw;
}

void bsTextManager::updateTexts(float deltaTime)
{
	std::for_each(mTextBoxes.begin(), mTextBoxes.end(),
		[deltaTime](const std::shared_ptr<bsScrollingText2D>& textObject)
	{
		textObject->update(deltaTime);
	});
}

void bsTextManager::toggleVisibility()
{
	std::for_each(mTexts.begin(), mTexts.end(),
		[](const std::shared_ptr<bsText2D>& textObject)
	{
			textObject->setEnabled(!textObject->getEnabled());
	});
}
