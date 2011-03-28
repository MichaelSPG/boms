#ifndef BSTEXTMANAGER_H
#define BSTEXTMANAGER_H

#include <string>
#include <vector>
#include <memory>

#include <FW1FontWrapper.h>

#include "bsText.h"

class Dx11Renderer;

class bsTextManager
{
public:
	bsTextManager(Dx11Renderer* dx11Renderer);
	~bsTextManager();
	
	std::shared_ptr<bsText> createText(const std::wstring& text,
		const std::wstring& font = L"Consolas");

	void destroyUnused();

	void drawAllTexts();
	
private:
	Dx11Renderer*	mDx11Renderer;
	IFW1Factory*	mFw1Factory;

	std::vector<std::shared_ptr<bsText>>	mTexts;
};

#endif // BSTEXTMANAGER_H