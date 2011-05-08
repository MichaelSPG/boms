#ifndef BS_CONSTANTBUFFERS_H
#define BS_CONSTANTBUFFERS_H

#include "bsConfig.h"

#include <Windows.h>
#include <xnamath.h>


struct CBWireFrame
{
	XMFLOAT4X4	world;
	XMFLOAT4	color;
};

struct CBWorld
{
	XMFLOAT4X4	world;
};

struct CBCamera
{
	XMFLOAT4X4	view;
	XMFLOAT4X4	projection;
	XMFLOAT4X4	viewProjection;
	float		farClip;
	float		unused[15];//multiple of 16
};

#endif // BS_CONSTANTBUFFERS_H
