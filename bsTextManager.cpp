#include "bsTextManager.h"

#include <assert.h>

#include "Dx11Renderer.h"
#include "Log.h"
#include "Templates.h"


bsTextManager::bsTextManager(Dx11Renderer* dx11Renderer)
	: mFw1Factory(nullptr)
	, mDx11Renderer(dx11Renderer)
{
	HRESULT hres = FW1CreateFactory(FW1_VERSION, &mFw1Factory);
	if (FAILED(hres))
	{
		Log::logMessage("Failed to create font factory", pantheios::SEV_CRITICAL);
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

std::shared_ptr<bsText> bsTextManager::createText(const std::wstring& text,
	const std::wstring& font /*= L"Consolas"*/)
{
	IFW1FontWrapper* fontWrapper;
	
	HRESULT hres = mFw1Factory->CreateFontWrapper(mDx11Renderer->getDevice(), font.c_str(),
		&fontWrapper);
	if (FAILED(hres))
	{
		std::string errorMessage("Failed to create font wrapper");
		Log::logMessage(errorMessage.c_str(), pantheios::SEV_ERROR);
	}

	assert(SUCCEEDED(hres) && "bsTextManager::createText failed");
	
	std::shared_ptr<bsText> textObject(new bsText(mDx11Renderer, text));
	//bsText* textObject = new bsText(mDx11Renderer, text);
	textObject->mFontWrapper = fontWrapper;

	mTexts.push_back(textObject);
	return textObject;
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

void bsTextManager::drawAllTexts()
{
	for (unsigned int i = 0u, count = mTexts.size(); i < count; ++i)
	{
		if (mTexts[i]->getEnabled())
		{
			mTexts[i]->draw();
		}
	}
}
