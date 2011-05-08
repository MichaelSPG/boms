#ifndef BS_VERTEX_TYPE_H
#define BS_VERTEX_TYPE_H

#include "bsConfig.h"
#include <Windows.h>
#include <xnamath.h>


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

#endif // BS_VERTEX_TYPE_H
