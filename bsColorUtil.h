#ifndef BS_COLORUTIL_H
#define BS_COLORUTIL_H

#include "bsConfig.h"
#include "bsMath.h"


/*	Class for color manipulation and conversions of colors.
	Uses XMFLOAT4 for 4x32 bit channels, where XYZW maps to RGBA.
	Uses unsigned int as a single object with 4x8 bit channels. Functions that deal with
	unsigned ints have both RGBA and ABGR vesions.
	RGBA maps to 0xRRGGBBAA, ABGR maps to 0xAABBGGRR.
*/
namespace bsColorUtil
{
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

/*	Converts 4 chars to an int containing the 4 chars, formatted as 0xAABBGGRR
*/
inline unsigned int toAbgr(unsigned char red, unsigned char green,
	unsigned char blue, unsigned char alpha)
{
	unsigned int res = red;
	res |= alpha << 24;
	res |= blue << 16;
	res |= green << 8;

	return res;
}

/*	Converts 4 chars to an int containing the 4 chars, formatted as 0xRRGGBBAA
*/
inline unsigned int toRgba(unsigned char red, unsigned char green,
	unsigned char blue, unsigned char alpha)
{
	unsigned int res = alpha;
	res |= red << 24;
	res |= green << 16;
	res |= blue << 8;

	return res;
}

/*	Returns a color specified by the offset from the RGBA input color.
	For example, 0x00FF0000 with offset set to red returns 0x00, but 0xFF with blue offset.
*/
inline unsigned char getColorFromRgba(unsigned int colorRgba, rgbaOffsets offset)
{
	unsigned char col;
	col = (unsigned char)(colorRgba >> offset);
	return col;
}

/*	Returns a color specified by the offset from the ABGR input color.
*/
inline unsigned char getColorFromAbgr(unsigned int colorAbgr, abgrOffsets offset)
{
	unsigned char col;
	col = (unsigned char)(colorAbgr >> offset);
	return col;
}

//Sets a new color value for one specified color
inline void modifyRgba(unsigned int& color, unsigned char newValue,
	rgbaOffsets offset)
{
	color |= newValue << offset;
}

inline void modifyAbgr(unsigned int& color, unsigned char newValue,
	abgrOffsets offset)
{
	color |= newValue << offset;
}

//RGBA to ABGR or ABGR to RGBA
inline void swapRgbaAbgr(unsigned int& color)
{
	//Color channels
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

/*	Converts 4 floats to an unsigned int.
	The input colors should be between 0.0 and 1.0.
*/
inline unsigned int toUintRgba(const XMFLOAT4& colorRgba)
{
	return toRgba((unsigned char)(colorRgba.x * 255.0f),
		(unsigned char)(colorRgba.y * 255.0f), (unsigned char)(colorRgba.z * 255.0f),
		(unsigned char)(colorRgba.w * 255.0f));
}

/*	Converts 4 floats to an unsigned int.
	The input colors should be between 0.0 and 1.0.
*/
inline unsigned int toUintAbgr(const XMFLOAT4& colorFloat)
{
	return toAbgr((unsigned char)(colorFloat.x * 255.0f),
		(unsigned char)(colorFloat.y * 255.0f), (unsigned char)(colorFloat.z * 255.0f),
		(unsigned char)(colorFloat.w * 255.0f));
}

/*	Converts an unsigned int to 4 floats
	The output will have colors between 0.0 and 1.0
*/
inline XMFLOAT4 toFloatFromRgba(unsigned int rgba)
{
	return XMFLOAT4(getColorFromRgba(rgba, rgbaOffsetRed),
		getColorFromRgba(rgba, rgbaOffsetGreen), getColorFromRgba(rgba, rgbaOffsetBlue),
		getColorFromRgba(rgba, rgbaOffsetAlpha));
}

/*	Converts an unsigned int to 4 floats
	The output will have colors between 0.0 and 1.0
*/
inline XMFLOAT4 toFloatFromAbgr(unsigned int abgr)
{
	return XMFLOAT4(getColorFromAbgr(abgr, abgrOffsetRed),
		getColorFromAbgr(abgr, abgrOffsetGreen), getColorFromAbgr(abgr, abgrOffsetBlue),
		getColorFromAbgr(abgr, abgrOffsetAlpha));
}


const unsigned int rgbaRed	= 0xff000000;
const unsigned int rgbaGreen = 0x00ff0000;
const unsigned int rgbaBlue = 0x0000ff00;

const unsigned int abgrRed	= 0x000000ff;
const unsigned int abgrGreen = 0x0000ff00;
const unsigned int abgrBlue = 0x00ff0000;
};

#endif // BS_COLORUTIL_H
