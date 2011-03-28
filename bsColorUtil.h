#ifndef BSCOLORUTIL_H
#define BSCOLORUTIL_H

#include "bsConfig.h"
#include "bsMath.h"


/*	Class for color manipulation and conversions.
	Uses XMFLOAT4 for 4x32 bit channels, where XYZW maps to RGBA.
	Uses unsigned int as a single object with 4x8 bit channels. Functions that deal with
	unsigned ints have RGBA and ABGR vesions.
	RGBA maps to 0xRRGGBBAA, ABGR maps to 0xAABBGGRR.
*/
class bsColorUtil
{
public:
	//Bit offsets for RGBA
	enum rgbaOffsets
	{
		rgbaOffsetRed	= 24,
		rgbaOffsetGreen	= 16,
		rgbaOffsetBlue	= 8,
		rgbaOffsetAlpha	= 0
	};

	//Bit offsets for ABGR
	enum abgrOffsets
	{
		abgrOffsetRed	= 0,
		abgrOffsetGreen	= 8,
		abgrOffsetBlue	= 16,
		abgrOffsetAlpha	= 24
	};

	//0xAABBGGRR
	inline static unsigned int toAbgr(unsigned char red, unsigned char green,
		unsigned char blue, unsigned char alpha)
	{
		unsigned int res = red;
		res |= alpha << 24;
		res |= blue << 16;
		res |= green << 8;

		return res;
	}

	//0xRRGGBBAA
	inline static unsigned int toRgba(unsigned char red, unsigned char green,
		unsigned char blue, unsigned char alpha)
	{
		unsigned int res = alpha;
		res |= red << 24;
		res |= green << 16;
		res |= blue << 8;

		return res;
	}

	inline static unsigned char getColorFromRgba(const unsigned int color,
		const rgbaOffsets offset)
	{
		unsigned char col;
		col = (unsigned char)(color >> offset);
		return col;

		//return color >> offset;
	}

	inline static unsigned char getColorFromAbgr(const unsigned int color,
		const abgrOffsets offset)
	{
		unsigned char col;
		col = (unsigned char)(color >> offset);
		return col;

		//return color >> offset;
	}

	//Sets a new color value for one specified color
	inline static void modifyRgba(unsigned int& color, unsigned char newValue,
		rgbaOffsets offset)
	{
		color |= newValue << offset;
	}

	inline static void modifyAbgr(unsigned int& color, unsigned char newValue,
		abgrOffsets offset)
	{
		color |= newValue << offset;
	}

	//RGBA to ABGR or ABGR to RGBA
	inline static void swapRgbaAbgr(unsigned int& color)
	{
		//Channels
		unsigned char c1 = (unsigned char)(color >> 24);
		unsigned char c2 = (unsigned char)(color >> 16);
		unsigned char c3 = (unsigned char)(color >> 8);
		unsigned char c4 = (unsigned char)(color >> 0);

		color = 0;
		color |= c1 << 0;
		color |= c2 << 8;
		color |= c3 << 16;
		color |= c4 << 24;
	}

	inline static unsigned int toUintRgba(const XMFLOAT4& colorFloat)
	{
		return toRgba((unsigned char)(colorFloat.x * 255.0f),
			(unsigned char)(colorFloat.y * 255.0f), (unsigned char)(colorFloat.z * 255.0f),
			(unsigned char)(colorFloat.w * 255.0f));
	}

	inline static unsigned int toUintAbgr(const XMFLOAT4& colorFloat)
	{
		return toAbgr((unsigned char)(colorFloat.x * 255.0f),
			(unsigned char)(colorFloat.y * 255.0f), (unsigned char)(colorFloat.z * 255.0f),
			(unsigned char)(colorFloat.w * 255.0f));
	}

	inline static XMFLOAT4 toFloatFromRgba(const unsigned int rgba)
	{
		return XMFLOAT4(getColorFromRgba(rgba, rgbaOffsetRed),
			getColorFromRgba(rgba, rgbaOffsetGreen), getColorFromRgba(rgba, rgbaOffsetBlue),
			getColorFromRgba(rgba, rgbaOffsetAlpha));
	}

	inline static XMFLOAT4 toFloatFromAbgr(const unsigned int abgr)
	{
		return XMFLOAT4(getColorFromAbgr(abgr, abgrOffsetRed),
			getColorFromAbgr(abgr, abgrOffsetGreen), getColorFromAbgr(abgr, abgrOffsetBlue),
			getColorFromAbgr(abgr, abgrOffsetAlpha));
	}


	static const unsigned int rgbaRed	= 0xff000000;
	static const unsigned int rgbaGreen = 0x00ff0000;
	static const unsigned int rgbaBlue = 0x0000ff00;

	static const unsigned int abgrRed	= 0x000000ff;
	static const unsigned int abgrGreen = 0x0000ff00;
	static const unsigned int abgrBlue = 0x00ff0000;
};

#endif // BSCOLORUTIL_H