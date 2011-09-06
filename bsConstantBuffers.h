#pragma once


#include <Windows.h>
#include <xnamath.h>


/*	Defines all the constant buffers used to represent transforms and other properties
	on the GPU.
*/

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
	XMFLOAT4X4	inverseViewProjection;
	XMFLOAT4	cameraPosition;
};

struct CBLight
{
	XMFLOAT4	lightPosition;
	XMFLOAT4	lightColor;
};
