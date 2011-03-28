#ifndef BSTEXT_H
#define BSTEXT_H

#include <string>

#include <FW1FontWrapper.h>

#include "bsMath.h"

class Dx11Renderer;

//Use bsTextManager to create bsText objects.
class bsText
{
	friend class bsTextManager;

	bsText(Dx11Renderer* dx11Renderer, const std::wstring& text);

public:
	~bsText();

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

	inline void setEnabled(const bool enabled)
	{
		mEnabled = enabled;
	}

	inline const XMFLOAT2& getPosition() const
	{
		return mPosition;
	}

	//Read/write access
	inline XMFLOAT2& getPosition()
	{
		return mPosition;
	}

	inline void setPosition(const XMFLOAT2& position)
	{
		mPosition = position;
	}

	inline void setPosition(const float x, const float y)
	{
		mPosition.x = x;
		mPosition.y = y;
	}

	inline float getFontSize() const
	{
		return mFontSize;
	}

	inline void setFontSize(const float size)
	{
		mFontSize = size;
	}

	inline unsigned int getColor()
	{
		return mColorAbgr;
	}

	inline void setColor(const unsigned int colorAbgr)
	{
		mColorAbgr = colorAbgr;
	}

	inline unsigned int getFlags() const
	{
		return mFlags;
	}

	inline void setFlags(const unsigned int flags)
	{
		mFlags = flags;
	}

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

#endif // BSTEXT_H