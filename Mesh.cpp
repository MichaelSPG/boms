#include "Mesh.h"

Mesh::Mesh()
	: mID(~0u)
	, mVertexBuffer(nullptr)
	, mIndexBuffer(nullptr)
	, mIndices(0)
{
}

Mesh::~Mesh()
{
	for (unsigned int i = 0u; i < mSubMeshes.size(); ++i)
	{
		delete mSubMeshes[i];
	}
	if (mVertexBuffer)
	{
		mVertexBuffer->Release();
	}
	if (mIndexBuffer)
	{
		mIndexBuffer->Release();
	}
}
