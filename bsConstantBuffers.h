#ifndef BS_CONSTANTBUFFERS_H
#define BS_CONSTANTBUFFERS_H

#include "bsConfig.h"

#include <Windows.h>
#include <xnamath.h>


/**	Defines all the constant buffers used to represent transforms and other properties
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
};

struct CBLight
{
	XMFLOAT3	color;
	float		radius;
	float		intensity;
	int			lightType;//bsLight::LightType
	int			padding[2];
};

#endif // BS_CONSTANTBUFFERS_H
