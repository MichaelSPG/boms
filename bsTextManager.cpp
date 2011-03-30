#include "bsTextManager.h"

#include <assert.h>

#include "bsDx11Renderer.h"
#include "bsLog.h"
#include "bsTemplates.h"
#include "bsTextBox.h"


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

std::shared_ptr<bsText2D> bsTextManager::createText(const std::wstring& text,
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

	assert(SUCCEEDED(hres) && "bsTextManager::createText failed");
	
	std::shared_ptr<bsText2D> textObject(new bsText2D(mDx11Renderer, text));
	//bsText2D* textObject = new bsText2D(mDx11Renderer, text);
	textObject->mFontWrapper = fontWrapper;

	mTexts.push_back(textObject);
	return textObject;
}

std::shared_ptr<bsTextBox> bsTextManager::createTextBox(const float fadeDelay,
	const unsigned int maxLineCount, const std::wstring& font /*= L"Consolas"*/)
{
	std::shared_ptr<bsTextBox> textBox(new bsTextBox(fadeDelay, maxLineCount));
	textBox->mText = createText(L"", font);

	mTextBoxes.push_back(textBox);

	return textBox;
}

void bsTextManager::destroyUnused()
{
	for (unsigned int i = 0u; i < mTexts.size(); ++i)
	{
		//Use count will be 1 if referenced only by this class
		if (mTexts[i].use_count() == 1)
		{
			unordered_erase(mTexts, mTexts[i]);

			--i;
		}
	}
}

void bsTextManager::drawAllTexts(const float deltaTime)
{
	for (unsigned int i = 0u, count = mTextBoxes.size(); i < count; ++i)
	{
		mTextBoxes[i]->update(deltaTime);
		mTextBoxes[i]->updateText();
	}

	for (unsigned int i = 0u, count = mTexts.size(); i < count; ++i)
	{
		if (mTexts[i]->getEnabled())
		{
			mTexts[i]->draw();
		}
	}
}
