#include "Node.h"

#include "Primitive.h"
#include "ShaderManager.h"


Node::~Node()
{
	if (mWireFramePrimitive)
	{
		delete mWireFramePrimitive;
	}
}

void Node::drawAABB(Dx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);
	assert(mWireFramePrimitive && "createDrawableAabb must be called before drawAABB");


	//m_color += 0.025f;
	//	m_color = XMFLOAT3(1/mDepth, -(1/mDepth), mDepth);


	CBWireFrame cb;
	XMStoreFloat4x4(&cb.world, XMMatrixIdentity());
	cb.world._14 = mPosition.getSimdAt(0);
	cb.world._24 = mPosition.getSimdAt(1);
	cb.world._34 = mPosition.getSimdAt(2);



	cb.color = XMFLOAT4(mWireframeColor.x, mWireframeColor.y, mWireframeColor.z, 0.0f);

	//cb.color = XMFLOAT4(-sin(m_color), sin(m_color), cos(m_color), 0.0f);
	//	float oneOverDepth = 1.0f / mDepth;
	/*
	if (mDepth > 3.1f)
	{
		cb.color = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
	}
	else if (mDepth > 2.1f)
	{
		cb.color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f);
	}
	else if (mDepth > 1.1f)
	{
		cb.color = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
	}
	else if (mDepth > 0.1f)
	{
		cb.color = XMFLOAT4(0.0f, 1.0f, 1.0f, 0.0f);
	}
	else if (mDepth < 0.2f)
	{
		cb.color = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);
	}
	*/

	dx11Renderer->getDeviceContext()->UpdateSubresource(mWireFramePrimitive->mBuffer, 0,
		nullptr, &cb, 0, 0);


	mWireFramePrimitive->draw(dx11Renderer);

	/*
	if (mChildren.size())
	{
		for (int i = 0; i < 8; ++i)
		{
			mChildren[i]->drawAABB(dx11Renderer);
		}
	}
	*/
}

void Node::createDrawableAabb(Dx11Renderer* dx11Renderer, ShaderManager* shaderManager)
{
	assert(dx11Renderer);

	mWireFramePrimitive = new Primitive();
	mWireFramePrimitive->createPrimitive(dx11Renderer, shaderManager, mAABB);

	/*
	if (mChildren.size())
	{
		for (int i = 0; i < 8; ++i)
		{
			mChildren[i]->createDrawableAabb(dx11Renderer, shaderManager);
		}
	}*/

//	m_color = (float)(rand() % mDepth);
}
