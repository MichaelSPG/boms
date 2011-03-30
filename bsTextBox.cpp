#include "bsTextBox.h"

#include <sstream>
#include <algorithm>

#include "bsText2D.h"
#include "bsStringUtils.h"


bsTextBox::bsTextBox(const float fadeDelay /*= 10000.0f*/, const unsigned int maxLineCount /*= 10*/)
	: mText(nullptr)
	, mMaxLineCount(maxLineCount)
	, mFadeDelay(fadeDelay)
{
	
}

bsTextBox::~bsTextBox()
{

}

void bsTextBox::addTextLine(const std::wstring& textLine)
{
	mTextLines.push_back(std::pair<std::wstring, float>(textLine, 0.0f));
	if (mTextLines.size() > mMaxLineCount)
	{
		mTextLines.pop_front();
	}
}

void bsTextBox::addTextLine(const std::string& textLine)
{
	addTextLine(bsStringUtils::utf8ToUtf16(textLine));
}

void bsTextBox::updateText()
{
	if (mText->getEnabled())
	{
		std::wstringstream text;
	
		for (auto itr = mTextLines.begin(), end = mTextLines.end(); itr != end; ++itr)
		{
			text << itr->first << L"\n";
		}

		mText->setText(text.str());
	}
}

void bsTextBox::update(const float deltaTime)
{
	if (mTextLines.empty())
	{
		return;
	}
	for (auto itr = mTextLines.begin(), end = mTextLines.end(); itr != end; ++itr)
	{
		itr->second += deltaTime;
	}

	//Remove the texts that have expired
	mTextLines.remove_if(
		[this](const std::pair<std::wstring, float>& text)
		{
			return text.second > mFadeDelay;
		});
}
