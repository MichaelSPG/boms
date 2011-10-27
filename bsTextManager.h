#pragma once

#include <string>
#include <vector>
#include <memory>

#include <FW1FontWrapper.h>

#include "bsText2D.h"

class bsDx11Renderer;
class bsScrollingText2D;
struct bsFrameStatistics;


/*	This class is responsible for creation and destruction of text objects.
	It is also used to render all the text objects.
*/
class bsTextManager
{
public:
	bsTextManager(bsDx11Renderer* dx11Renderer);
	~bsTextManager();
	
	/*	Creates a 2D text element with the specified text and font.
		See bsText2D for more info.
	*/
	std::shared_ptr<bsText2D> createText2D(const std::wstring& text,
		const std::wstring& font = L"Consolas");

	/*	Creates a 2D scrolling text element with the specified font.
		The fade delay specifies for how long each text line will stay visible.
		If the number of active text lines exceeds the max line count, the oldest
		text line will be removed.

		See bsScrollingText2D for more info.
	*/
	std::shared_ptr<bsScrollingText2D> createScrollingText2D(float fadeDelay,
		unsigned int maxLineCount, const std::wstring& font = L"Consolas");

	/*	Deletes every text object that is not referenced anywhere else.		
	*/
	void destroyUnusedTexts();

	/*	Renders all the text objects to the screen (except for ones that are disabled).		
	*/
	void drawAllTexts(bsFrameStatistics& frameStatistics);

	/*	Updates all the texts, making them scroll or applies other effects if applicable.
	*/
	void updateTexts(float deltaTime);

	/*	Toggles the visibility of every text object.		
	*/
	void toggleVisibility();

	inline IFW1Factory* getFw1Factory() const
	{
		return mFw1Factory;
	}
	

private:
	bsDx11Renderer*	mDx11Renderer;
	IFW1Factory*	mFw1Factory;

	std::vector<std::shared_ptr<bsText2D>>			mTexts;
	std::vector<std::shared_ptr<bsScrollingText2D>>	mTextBoxes;
};
