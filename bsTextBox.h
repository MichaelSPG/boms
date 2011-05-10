#ifndef BS_TEXTBOX_H
#define BS_TEXTBOX_H

#include "bsConfig.h"

#include <list>
#include <string>
#include <memory>

#include <FW1FontWrapper.h>

class bsText2D;


class bsTextBox
{
	friend class bsTextManager;

	bsTextBox(float fadeDelay = 10000.0f, unsigned int maxLineCount = 10);

public:
	~bsTextBox();

	//Will be treated as a single line of text regardless of newlines.
	void addTextLineWide(const std::wstring& textLine);

	void addTextLine(const std::string& textLine);
	
	inline std::shared_ptr<bsText2D> getText() const
	{
		return mText;
	}

private:
	void update(float deltaTime);

	void updateText();

	std::shared_ptr<bsText2D>	mText;
	unsigned int				mMaxLineCount;
	//pair<string, lifeTimeMs>
	std::list<std::pair<std::wstring, float>>		mTextLines;

	float	mFadeDelay;
};

#endif // BS_TEXTBOX_H
