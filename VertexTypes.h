#ifndef VERTEX_TYPE_H
#define VERTEX_TYPE_H

#define SVertexType D3DFVF_XYZRHW | D3DFVF_DIFFUSE

class SVertex
{
public:
	float x, y, z, w;

	//Diffuse color
	DWORD color;
};

#endif