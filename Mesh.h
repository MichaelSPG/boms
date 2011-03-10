#ifndef MESH_H
#define MESH_H

#include "Dx11Renderer.h"


class Mesh
{
	friend class MeshManager;

	Mesh();

public:
	~Mesh();

	void draw(Dx11Renderer* dx11Renderer)
	{
		if (mVertexBuffer && mIndices)
		{
			auto context = dx11Renderer->getDeviceContext();

			UINT offsets =  0;
			UINT stride = sizeof(VertexNormal);
			context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offsets);
			context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

			//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			context->DrawIndexed(mIndices, 0, 0);
		}

		for (unsigned int i = 0u; i < mSubMeshes.size(); ++i)
		{
			mSubMeshes[i]->draw(dx11Renderer);
		}
	}


private:	
	ID3D11Buffer*		mVertexBuffer;
	ID3D11Buffer*		mIndexBuffer;
	unsigned int		mIndices;
	std::vector<Mesh*>	mSubMeshes;

	unsigned int	mID;
};

#endif // MESH_H