#ifndef BS_TEXTMANAGER_H
#define BS_TEXTMANAGER_H

#include <string>
#include <vector>
#include <memory>

#include <FW1FontWrapper.h>

#include "bsText2D.h"

class bsDx11Renderer;
class bsTextBox;

class bsTextManager
{
public:
	bsTextManager(bsDx11Renderer* dx11Renderer);
	~bsTextManager();
	
	std::shared_ptr<bsText2D> createText(const std::wstring& text,
		const std::wstring& font = L"Consolas");

	std::shared_ptr<bsTextBox> createTextBox(const float fadeDelay,
		const unsigned int maxLineCount, const std::wstring& font = L"Consolas");

	void destroyUnused();

	void drawAllTexts();

	//Updates all the texts, making them scroll or other effects if applicable.
	void updateTexts(float deltaTime);
	
private:
	bsDx11Renderer*	mDx11Renderer;
	IFW1Factory*	mFw1Factory;

	std::vector<std::shared_ptr<bsText2D>>	mTexts;
	std::vector<std::shared_ptr<bsTextBox>>	mTextBoxes;
};

#endif // BS_TEXTMANAGER_H
