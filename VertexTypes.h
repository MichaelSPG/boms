#ifndef VERTEX_TYPE_H
#define VERTEX_TYPE_H

#define SVertexType D3DFVF_XYZRHW | D3DFVF_DIFFUSE

#include <assimp.hpp>

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
	aiVector3D position;
	aiVector3D normal;
};

//Vertex position, normal, texture coordinates
struct VertexNormalTex
{
	aiVector3D position;
	aiVector3D normal;
	aiVector2D texCoord;
};

#endif