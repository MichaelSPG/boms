#include "OctNode.h"
#include "Log.h"
#include "SceneGraph.h"
#include "Dx11Renderer.h"
#include "AABB.h"

OctNode::OctNode(int id, const XMFLOAT3& position, SceneGraph* sceneGraph,
	OctNode* parent, const AABB& aabb)
	: Node(position, id, sceneGraph)
	, mParent(parent)
	, mAABB(new AABB(aabb))
	, mWireFramePrimitive(nullptr)
{
	/*
	std::ostringstream out;
	out << "Creating a node, ID: " << mID;
	Log::log(out.str().c_str());
	*/

	/*if (!parent)
	{
		mAABB = AABB(0.0f, 0.0f, 0.0f, 1000.0f, 1000.0f, 1000.0f);
	}
	else
	{
		XMVectorGetX(parent->mAABB.mPosition) - XMVectorGetX(parent->mAABB.mHalfExtents);
		
		mAABB = AABB(XMVectorGetX(parent->mAABB.mPosition) - XMVectorGetX(parent->mAABB.mHalfExtents),
					 XMVectorGetY(parent->mAABB.mPosition) - XMVectorGetY(parent->mAABB.mHalfExtents),
					 XMVectorGetZ(parent->mAABB.mPosition) - XMVectorGetZ(parent->mAABB.mHalfExtents),
					 1,1,1);
		
	}*/
}

OctNode::~OctNode()
{
	/*
	std::ostringstream out;
	out << "Deleting a node, ID: " << mID;
	Log::log(out.str().c_str());
	*/

	if (mWireFramePrimitive)
		delete mWireFramePrimitive;
}

void OctNode::createChildren(unsigned short depth, SceneGraph* sceneGraph)
{
	if (--depth == 0)
		return;

	mChildren.reserve(8);
	
	//Each child node's extents needs to be exactly half the size of its parent.
	XMFLOAT3 halfHalfExtents = mAABB->mHalfExtents;
	halfHalfExtents.x *= 0.5f;
	halfHalfExtents.y *= 0.5f;
	halfHalfExtents.z *= 0.5f;

	AABB aabb(halfHalfExtents, this);

	//Create 8 children and make their AABBs fill the current node's AABB.

	XMFLOAT3 position;
	for (int i = 0; i < 2; ++i)
	{
		if (i == 0)
		{
			position.x = mPosition.x + halfHalfExtents.x;
		}
		else
		{
			position.x = mPosition.x - halfHalfExtents.x;
		}

		for (int j = 0; j < 2; ++j)
		{
			if (j == 0)
			{
				position.y = mPosition.y + halfHalfExtents.y;
			}
			else
			{
				position.y = mPosition.y - halfHalfExtents.y;
			}

			for (int k = 0; k < 2; ++k)
			{
				if (k == 0)
				{
					position.z = mPosition.z + halfHalfExtents.z;
				}
				else
				{
					position.z = mPosition.z - halfHalfExtents.z;
				}

				mChildren.push_back(std::make_shared<OctNode>(sceneGraph->
					getNumCreatedObjects(), position, mSceneGraph, this, aabb));
			}
		}
	}
	/*
	for (int i = 0; i < 8; ++i)
	{
		XMFLOAT3 position;

		if (i % 2 == 1)
		{
			position.x = mPosition.x + halfHalfExtents.x;
		}
		else
		{
			position.x = mPosition.x - halfHalfExtents.x;
		}

		switch (i % 4)
		{
		case 0:
			position.y = mPosition.y + halfHalfExtents.y;
			break;

		case 1:
			position.y = mPosition.y + halfHalfExtents.y;
			break;

		case 2:
			position.z = mPosition.z + halfHalfExtents.z;
			break;

		case 3:
			position.z = mPosition.z - halfHalfExtents.z;
			break;
		}
		
	//	position.x = position.y = position.z = 0.0f;
		mChildren.push_back(std::make_shared<OctNode>(sceneGraph->getNumCreatedObjects(),
			position, mSceneGraph, this, aabb));
	}
	*/
	for (int i = 0; i < 8; ++i)
	{
		mChildren[i]->createChildren(depth, sceneGraph);
	}
}

void OctNode::attachSceneNode(std::shared_ptr<SceneNode> node)
{
	mSceneNodes.push_back(node);
}

void OctNode::drawAABB(Dx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);

	m_color += 0.025f;


	CBWireFrame cb;
	XMStoreFloat4x4(&cb.world, XMMatrixIdentity());
	cb.world._14 = mPosition.x;
	cb.world._24 = mPosition.y;
	cb.world._34 = mPosition.z;
	
	cb.color = XMFLOAT4(-sin(m_color), sin(m_color), cos(m_color), 0.0f);

	dx11Renderer->getDeviceContext()->UpdateSubresource(mWireFramePrimitive->mBuffer, 0,
		nullptr, &cb, 0, 0);


	mWireFramePrimitive->draw(dx11Renderer);

	if (mChildren.size())
	{
		for (int i = 0; i < 8; ++i)
		{
			mChildren[i]->drawAABB(dx11Renderer);
		}
	}
}

void OctNode::prepareForDrawing(Dx11Renderer* dx11Renderer, ShaderManager* shaderManager)
{
	assert(dx11Renderer);

	mWireFramePrimitive = new Primitive();
	mWireFramePrimitive->createPrimitive(dx11Renderer, shaderManager, *mAABB);

	if (mChildren.size())
	{
		for (int i = 0; i < 8; ++i)
		{
			mChildren[i]->prepareForDrawing(dx11Renderer, shaderManager);
		}
	}

	m_color = (float)rand();
}
