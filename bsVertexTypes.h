#pragma once

#include "bsConfig.h"
#include <Windows.h>
#include <xnamath.h>

/*	Defines all the various vertex types.
	A textured mesh requires different properties than a wireframe primitive,
	so one vertex type does not work for everything.
*/


//Vertex with texture coordinate
struct SimpleVertex 
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

//Vertex with normal
struct VertexNormal
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
};

//Vertex with texture coordinates
struct VertexTex
{
	XMFLOAT3 position;
	XMFLOAT2 textureCoord;
};

//Vertex position, normal, texture coordinates
struct VertexNormalTex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 textureCoord;
};
