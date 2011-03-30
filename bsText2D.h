#ifndef BS_TEXT_H
#define BS_TEXT_H

#include <string>

#include <FW1FontWrapper.h>

#include "bsMath.h"

class bsDx11Renderer;

//Use bsTextManager to create bsText2D objects.
class bsText2D
{
	friend class bsTextManager;

	bsText2D(bsDx11Renderer* dx11Renderer, const std::wstring& text);

public:
	~bsText2D();

	inline IFW1FontWrapper* getFontWrapper() const
	{
		return mFontWrapper;
	}

	inline const std::wstring& getText() const
	{
		return mText;
	}

	//Read/write access
	inline std::wstring& getText()
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

	//Default: true
	inline void setEnabled(const bool enabled)
	{
		mEnabled = enabled;
	}

	//Read only
	inline const XMFLOAT2& getPosition() const
	{
		return mPosition;
	}

	//Read/write access
	inline XMFLOAT2& getPosition()
	{
		return mPosition;
	}

	//Default: 0.0f, 0.0f
	inline void setPosition(const XMFLOAT2& position)
	{
		mPosition = position;
	}

	//Default: 0.0f, 0.0f
	inline void setPosition(const float x, const float y)
	{
		mPosition.x = x;
		mPosition.y = y;
	}

	inline float getFontSize() const
	{
		return mFontSize;
	}

	//Default: 12.0f
	inline void setFontSize(const float size)
	{
		mFontSize = size;
	}

	inline unsigned int getColor()
	{
		return mColorAbgr;
	}

	//Default: 0xFFFFFFFF (white)
	inline void setColor(const unsigned int colorAbgr)
	{
		mColorAbgr = colorAbgr;
	}

	//See FW1_TEXT_FLAG enum
	inline unsigned int getFlags() const
	{
		return mFlags;
	}

	//See FW1_TEXT_FLAG enum
	//Default: 0
	inline void setFlags(const unsigned int flags)
	{
		mFlags = flags;
	}

	//See FW1_TEXT_FLAG enum
	//Default: 0
	inline void addFlags(const unsigned int flags)
	{
		mFlags |= flags;
	}

	void draw();
	
private:
	bool					mEnabled;
	std::wstring			mText;
	IFW1FontWrapper*		mFontWrapper;
	ID3D11DeviceContext*	mDeviceContext;

	unsigned int	mColorAbgr;
	float			mFontSize;
	XMFLOAT2		mPosition;
	unsigned int	mFlags;
};

#endif // BS_TEXT_H
