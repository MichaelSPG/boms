#include "OctNode.h"

#include "Log.h"
#include "SceneGraph.h"
#include "Dx11Renderer.h"
#include "Convert.h"


OctNode::OctNode(int id, const hkVector4& position, SceneGraph* sceneGraph,
	OctNode* parent, const hkAabb& aabb)
	: Node(position, id, sceneGraph)
	, mParent(parent)
{
	mAABB = aabb;

	mWireframeColor = XMFLOAT3(0.25f, 0.25f, 0.25f);
}

OctNode::~OctNode()
{
	while (mChildren.size())
	{
		delete mChildren.back();
		mChildren.pop_back();
	}
}

void OctNode::createChildren(unsigned short depth, SceneGraph* sceneGraph)
{
	++depth;
	if (depth >= mSceneGraph->mMaxTreeDepth)
		return;

	mDepth = depth;

	mChildren.reserve(8u);
	
	//Each child node's extents needs to be exactly half the size of its parent.
	hkVector4 aabbMin;
	hkVector4 aabbMax;
	aabbMin.setMul4(0.5f, mAABB.m_min);
	aabbMax.setMul4(0.5f, mAABB.m_max);

	hkAabb aabb(aabbMin, aabbMax);
	
	hkVector4 halfHalfExtents;
	aabb.getHalfExtents(halfHalfExtents);
	
	aabb.m_min.sub3clobberW(halfHalfExtents);
	aabb.m_max.sub3clobberW(halfHalfExtents);

	//Create 8 children and make their AABBs fill the current node's AABB.

	hkVector4 position(0.0f, 0.0f, 0.0f);
	hkQuadReal& positionQuad = position.getQuad();

	for (int i = 0; i < 2; ++i)
	{
		if (i == 0)
		{
			positionQuad.x = mPosition.getSimdAt(0) + halfHalfExtents.getSimdAt(0);
		}
		else
		{
			positionQuad.x = mPosition.getSimdAt(0) - halfHalfExtents.getSimdAt(0);
		}

		for (int j = 0; j < 2; ++j)
		{
			if (j == 0)
			{
				positionQuad.y = mPosition.getSimdAt(1) + halfHalfExtents.getSimdAt(1);
			}
			else
			{
				positionQuad.y = mPosition.getSimdAt(1) - halfHalfExtents.getSimdAt(1);
			}

			for (int k = 0; k < 2; ++k)
			{
				if (k == 0)
				{
					positionQuad.z = mPosition.getSimdAt(2) + halfHalfExtents.getSimdAt(2);
				}
				else
				{
					positionQuad.z = mPosition.getSimdAt(2) - halfHalfExtents.getSimdAt(2);
				}

				OctNode* node = new OctNode(sceneGraph->getNumCreatedObjects(),
					hkVector4(0.0f, 0.0f, 0.0f, 0.0f), mSceneGraph, this, aabb);

				node->setPosition(position);

				mChildren.push_back(node);
				mSceneGraph->mOctNodes.push_back(node);
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
