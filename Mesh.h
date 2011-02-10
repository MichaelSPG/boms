#ifndef MESH_H
#define MESH_H

#include "Dx11Renderer.h"

class Mesh
{
public:
	Mesh(SimpleVertex *vertices, WORD *indices, Dx11Renderer *renderer);

	~Mesh()
	{
		mVertexBuffer->Release();
		mIndexBuffer->Release();
	}

private:	
	ID3D11Buffer		*mVertexBuffer;
	ID3D11Buffer		*mIndexBuffer;
	
};

#endif // MESH_H