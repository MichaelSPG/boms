#pragma once

#include <string>

#include <xnamath.h>

#include "bsCollision.h"

struct ID3D11DeviceContext;
struct ID3D11Device;
struct IFW1FontWrapper;
struct IFW1Factory;


class bsText3D
{
public:
	bsText3D(ID3D11DeviceContext* deviceContext, ID3D11Device* device, IFW1Factory* fw1Factory);

	~bsText3D();

	inline void setText(const std::wstring& text)
	{
		mText = text;
	}

	void draw(const XMMATRIX& transform) const;

	bsCollision::Sphere getBoundingSphere() const;


private:
	IFW1FontWrapper*		mFontWrapper;
	ID3D11DeviceContext*	mDeviceContext;

	std::wstring	mText;
	std::wstring	mFont;
	float			mFontSize;
	unsigned int	mColorAabbggrr;
};
