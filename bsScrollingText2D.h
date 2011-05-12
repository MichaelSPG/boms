#ifndef BS_SCROLLINGTEXT2D_H
#define BS_SCROLLINGTEXT2D_H

#include "bsConfig.h"

#include <deque>
#include <string>
#include <memory>

#include <FW1FontWrapper.h>

class bsText2D;


/*	This class can be used to create a text object with scrolling text.
	Individual text lines can be added to a FIFO container and will be rendered until
	their lifetimes are greater than the set max lifetime.
	
*/
class bsScrollingText2D
{
	friend class bsTextManager;

	bsScrollingText2D(float fadeDelay = 10000.0f, unsigned int maxLineCount = 10);

	struct TextLine
	{
		TextLine(const std::wstring& txt)
			: text(txt)
			, currentLifeMs(0.0f)
		{}
		TextLine()
			: currentLifeMs(0.0f)
		{}

		std::wstring	text;
		float			currentLifeMs;
	};

public:
	~bsScrollingText2D();

	/*	Add a single line of text to the FIFO container.
		You can pass a string with newlines to this function, but it will be treated as
		a single line regardless.
	*/
	void addTextLineWide(const std::wstring& textLine);

	void addTextLine(const std::string& textLine);
	
	/*	Returns the internal text object used, use this to modify flags or other
		properties if required.
	*/
	inline std::shared_ptr<bsText2D> getText() const
	{
		return mText;
	}

private:
	/*	Copies all the stored text lines into the underlying bsText2D element,
		resulting in it not containing expired text lines.
	*/
	void updateText();

	/*	Finds out if any texts have expired, and if so, removes them and updates the
		underlying bsText2D element.		
	*/
	void update(float deltaTime);

	std::shared_ptr<bsText2D>	mText;
	unsigned int				mMaxLineCount;
	std::deque<TextLine>		mTextLines;

	float	mFadeDelay;
	bool	mNeedTextSync;
};

#endif // BS_SCROLLINGTEXT2D_H
