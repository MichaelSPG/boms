#pragma once


#include <Windows.h>
#include <xnamath.h>


/*	Defines all the constant buffers used to represent transforms and other properties
	on the GPU.
*/

__declspec(align(16)) struct CBWireFrame
{
	XMMATRIX	world;
	XMFLOAT4A	color;
};

__declspec(align(16)) struct CBWorld
{
	XMVECTOR	world;
};

__declspec(align(16)) struct CBCamera
{
	XMMATRIX	view;
	XMMATRIX	projection;
	XMMATRIX	viewProjection;
	XMMATRIX	inverseViewProjection;
	XMVECTOR	cameraPosition;
};

__declspec(align(16)) struct CBLight
{
	XMFLOAT4A	lightPosition;
	XMFLOAT4A	lightColor;
};

struct CBFxaa
{
	XMFLOAT2	oneOverScreenSize;
	XMFLOAT2	padding;//Pad to multiple of 16 bytes.
};