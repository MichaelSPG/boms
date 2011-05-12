#ifndef BS_TEXT2D_H
#define BS_TEXT2D_H

#include <string>

#include <FW1FontWrapper.h>

#include "bsMath.h"

class bsDx11Renderer;


/*	A 2D text objects which can be used to show text on the screen.
	The text contained by this object can be changed at any time during runtime.
	
	Use bsTextManager to create bsText2D objects.
*/
class bsText2D
{
	friend class bsTextManager;

	bsText2D(bsDx11Renderer* dx11Renderer, const std::wstring& text);

public:
	~bsText2D();

	inline const std::wstring& getText() const
	{
		return mText;
	}

	inline void setText(const std::wstring& text)
	{
		mText = text;
	}

	inline bool getEnabled() const
	{
		return mEnabled;
	}

	/*	If a text is not enabled, if will not be drawn.
		Default: true
	*/
	inline void setEnabled(bool enabled)
	{
		mEnabled = enabled;
	}


	inline const XMFLOAT2& getPosition() const
	{
		return mPosition;
	}

	//Default: 0.0f, 0.0f
	inline void setPosition(const XMFLOAT2& position)
	{
		mPosition = position;
	}

	//Default: 0.0f, 0.0f
	inline void setPosition(float x, float y)
	{
		mPosition.x = x;
		mPosition.y = y;
	}

	inline float getFontSize() const
	{
		return mFontSize;
	}

	//Default: 12.0f
	inline void setFontSize(float size)
	{
		mFontSize = size;
	}

	inline unsigned int getColor()
	{
		return mColorAbgr;
	}

	//Default: 0xFFFFFFFF (white)
	inline void setColor(unsigned int colorAbgr)
	{
		mColorAbgr = colorAbgr;
	}

	//See FW1_TEXT_FLAG enum
	inline unsigned int getFlags() const
	{
		return mFlags;
	}

	/*	See FW1_TEXT_FLAG enum
		Default: 0 (left aligned)
	*/
	inline void setFlags(unsigned int flags)
	{
		mFlags = flags;
	}

	/*	See FW1_TEXT_FLAG enum
		Default: 0 (left aligned)
		Bitwise ORs one or multiple flags, i.e. current |= flags
	*/
	inline void addFlags(unsigned int flags)
	{
		mFlags |= flags;
	}

	
private:
	//Renders the text to active render target
	void draw();

	bool					mEnabled;
	std::wstring			mText;
	IFW1FontWrapper*		mFontWrapper;
	ID3D11DeviceContext*	mDeviceContext;

	unsigned int	mColorAbgr;
	float			mFontSize;
	XMFLOAT2		mPosition;
	unsigned int	mFlags;
};

#endif // BS_TEXT2D_H
