#ifndef BS_VERTEX_TYPE_H
#define BS_VERTEX_TYPE_H

#include "bsConfig.h"
#include "bsMath.h"


//Vertex with normal
struct VertexNormal
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
};

//Vertex position, normal, texture coordinates
struct VertexNormalTex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 texCoord;
};

#endif // BS_VERTEX_TYPE_H
