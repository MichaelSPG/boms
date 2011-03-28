#ifndef VERTEX_TYPE_H
#define VERTEX_TYPE_H

#include "bsConfig.h"
#include "bsMath.h"

#define SVertexType D3DFVF_XYZRHW | D3DFVF_DIFFUSE


class SVertex
{
public:
	float x, y, z, w;

	//Diffuse color
	DWORD color;
};

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

#endif