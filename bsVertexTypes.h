#pragma once


#include <Windows.h>
#include <xnamath.h>

/*	Defines all the various vertex types.
	A textured mesh requires different properties than a wireframe primitive,
	so one vertex type does not work for everything.
*/


//Vertex with texture coordinate
struct bsSimpleVertex 
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

//Vertex with normal
struct bsVertexNormal
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
};

//Vertex with texture coordinates
struct bsVertexTex
{
	XMFLOAT3 position;
	XMFLOAT2 textureCoord;
};

//Vertex position, normal, texture coordinates
struct bsVertexNormalTex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 textureCoord;
};

//Vertex position, normal, tangent, texture coordinates
struct bsVertexNormalTangentTex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT2 textureCoord;
};

/*	A vertex buffer, with an array of vertices, normals and texture coordinates.
*/
struct bsVertexBuffer
{
	bsVertexNormalTangentTex*	vertices;
	unsigned int		vertexCount;
};

/*	An index buffer with an array of uints.
*/
struct bsIndexBuffer
{
	unsigned int*	indices;
	unsigned int	indexCount;
};
