#include "bsTextManager.h"

#include <cassert>

#include "bsDx11Renderer.h"
#include "bsLog.h"
#include "bsTemplates.h"
#include "bsScrollingText2D.h"


bsTextManager::bsTextManager(bsDx11Renderer* dx11Renderer)
	: mFw1Factory(nullptr)
	, mDx11Renderer(dx11Renderer)
{
	HRESULT hres = FW1CreateFactory(FW1_VERSION, &mFw1Factory);
	if (FAILED(hres))
	{
		bsLog::logMessage("Failed to create font factory", pantheios::SEV_CRITICAL);
	}
	
	assert(SUCCEEDED(hres) && "bsTextManager::bsTextManager font factory creation");
}

bsTextManager::~bsTextManager()
{
	if (mFw1Factory)
	{
		mFw1Factory->Release();
	}
}

std::shared_ptr<bsText2D> bsTextManager::createText2D(const std::wstring& text,
	const std::wstring& font /*= L"Consolas"*/)
{
	IFW1FontWrapper* fontWrapper;
	
	HRESULT hres = mFw1Factory->CreateFontWrapper(mDx11Renderer->getDevice(), font.c_str(),
		&fontWrapper);
	if (FAILED(hres))
	{
		std::string errorMessage("Failed to create font wrapper");
		bsLog::logMessage(errorMessage.c_str(), pantheios::SEV_ERROR);
	}

	assert(SUCCEEDED(hres) && "bsTextManager::createText2D failed");
	
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

void bsTextManager::drawAllTexts()
{
	for (unsigned int i = 0, count = mTexts.size(); i < count; ++i)
	{
		mTexts[i]->draw();
	}
}

void bsTextManager::updateTexts(float deltaTime)
{
	for (unsigned int i = 0, count = mTextBoxes.size(); i < count; ++i)
	{
		mTextBoxes[i]->update(deltaTime);
	}
}
