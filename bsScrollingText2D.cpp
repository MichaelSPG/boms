#include "bsScrollingText2D.h"

#include <sstream>
#include <algorithm>

#include "bsText2D.h"
#include "bsStringUtils.h"
#include "bsTemplates.h"


bsScrollingText2D::bsScrollingText2D(float fadeDelay /*= 10000.0f*/,
	unsigned int maxLineCount /*= 10*/)
	: mText(nullptr)
	, mMaxLineCount(maxLineCount)
	, mFadeDelay(fadeDelay)
	, mTextLines(maxLineCount + 1)//+1 to account for possible extra line after an add
	, mNeedTextSync(false)
{
}

bsScrollingText2D::~bsScrollingText2D()
{
}

void bsScrollingText2D::addTextLineWide(const std::wstring& textLine)
{
	mTextLines.push_back(TextLine(textLine));
	while (mTextLines.size() > mMaxLineCount)
	{
		mTextLines.pop_front();
	}

	mNeedTextSync = true;
}

void bsScrollingText2D::addTextLine(const std::string& textLine)
{
	addTextLineWide(bsStringUtils::utf8ToUtf16(textLine));
}

void bsScrollingText2D::updateText()
{
	if (mText->getEnabled())
	{
		std::wstringstream text;
		
		//Iterate through all the currently stored texts and generate a single string
		//containing all of them
		for (unsigned int i = 0, count = mTextLines.size(); i < count; ++i)
		{
			text << mTextLines[i].text << L"\n";
		}
		
		mText->setText(text.str());
	}

	mNeedTextSync = false;
}

void bsScrollingText2D::update(float deltaTime)
{
	if (mTextLines.empty())
	{
		return;
	}

	//Increase lifetime for all texts
	for (unsigned int i = 0, count = mTextLines.size(); i < count; ++i)
	{
		mTextLines[i].currentLifeMs += deltaTime;
	}

	//Remove expired texts. The one at the front will always be oldest.
	while (!mTextLines.empty() && mTextLines.front().currentLifeMs > mFadeDelay)
	{
		mTextLines.pop_front();
		mNeedTextSync = true;
	}
	
	if (mNeedTextSync)
	{
		//The collection of text lines was changed, need to sync changes
		updateText();
	}
}
