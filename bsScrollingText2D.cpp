#include "StdAfx.h"

#include "bsScrollingText2D.h"

#include <algorithm>
#include <string>

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
		//Iterate through all elements once to find out how much space needs to be
		//allocated to combine them all.
		size_t totalRequiredSize = 0;
		for (unsigned int i = 0; i < mTextLines.size(); ++i)
		{
			totalRequiredSize += mTextLines[i].text.length();
		}

		std::wstring text;
		//Adding number of text lines to account for newlines.
		text.reserve(totalRequiredSize + mTextLines.size());

		//Iterate through all the currently stored texts and generate a single string
		//containing all of them
		for (unsigned int i = 0; i < mTextLines.size(); ++i)
		{
			text.append(mTextLines[i].text);
			text.push_back(L'\n');
		}

		mText->setText(text);
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
