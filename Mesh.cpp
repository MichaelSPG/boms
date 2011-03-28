#include "Mesh.h"

#include "ShaderManager.h"


Mesh::Mesh()
	: mID(~0u)
	, mVertexBuffer(nullptr)
	, mIndexBuffer(nullptr)
	, mIndices(0)
	, mAabb(hkVector4(0.0f, 0.0f, 0.0f, 0.0f), hkVector4(0.0f, 0.0f, 0.0f, 0.0f))
	, mDrawableAabbActive(false)
	, mAabbPrimitive(nullptr)
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
	if (mAabbPrimitive)
	{
		delete mAabbPrimitive;
	}
}

void Mesh::draw(Dx11Renderer* dx11Renderer) const
{
	if (mVertexBuffer && mIndices)
	{
		auto context = dx11Renderer->getDeviceContext();

		UINT offsets =  0;
		UINT stride = sizeof(VertexNormalTex);
		context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offsets);
		context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->DrawIndexed(mIndices, 0, 0);
	}

	for (unsigned int i = 0u; i < mSubMeshes.size(); ++i)
	{
		mSubMeshes[i]->draw(dx11Renderer);
	}
	if (mDrawableAabbActive)
	{
	//	mAabbPrimitive->draw(dx11Renderer);
	}
}

void Mesh::updateAABB()
{
	for (unsigned int i = 0u; i < mSubMeshes.size(); ++i)
	{
		mSubMeshes[i]->updateAABB();

		const hkAabb& subMeshAabb = mSubMeshes[i]->mAabb;

		if (!mAabb.contains(subMeshAabb))
		{
			mAabb.includeAabb(subMeshAabb);
		}
	}
}

void Mesh::createDrawableAabb(Dx11Renderer* dx11Renderer, ShaderManager* shaderManager)
{
	mAabbPrimitive = new Primitive();
	mAabbPrimitive->createPrimitive(dx11Renderer, shaderManager, mAabb, false);

	mDrawableAabbActive = true;
}

bool Mesh::operator<(const Mesh& other)
{
	if (other.mID < mID)
	{
		return true;
	}
	//else if (other.mID > mID)
	else
	{
		return false;
	}
//	else
	{
//		return other.mSceneNode < mSceneNode;
	}
}
