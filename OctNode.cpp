#include "OctNode.h"

#include <assert.h>

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
	mDepth = depth;

	if (depth >= mSceneGraph->mMaxTreeDepth)
	{
		return;
	}

	mChildren.reserve(8u);

	//Each child node's extents needs to have extents exactly half the size of its parent.
	hkVector4 halfExtents;
	mAABB.getHalfExtents(halfExtents);
	halfExtents.mul4(0.5f);
	//halfExtents now represents the max point for the new AABB.

	//Make a negative of halfExtents so that we have both min and max points for the new AABB.
	hkVector4 halfExtentsNeg;
	halfExtentsNeg.setNeg3(halfExtents);

	//If half extents are 20, 20, 20, this AABB will have halfExtentsNeg equal -10, -10, -10
	//and halfExtents equal 10, 10, 10.
	const hkAabb aabb(halfExtentsNeg, halfExtents);
	

	hkVector4 position(0.0f, 0.0f, 0.0f);
	float halfExtentX = halfExtents.getSimdAt(0);

	//If this is false, the AABB is not square and this function needs to be altered,
	//for instance to getting x,y,z manually each time instead of using halfExtentX.

	assert(halfExtents.getSimdAt(0) == halfExtents.getSimdAt(1)
		&& halfExtents.getSimdAt(0) == halfExtents.getSimdAt(2)
		&& halfExtents.getSimdAt(1) == halfExtents.getSimdAt(2));

	assert(halfExtentsNeg.getSimdAt(0) == halfExtentsNeg.getSimdAt(1)
		&& halfExtentsNeg.getSimdAt(0) == halfExtentsNeg.getSimdAt(2)
		&& halfExtentsNeg.getSimdAt(1) == halfExtentsNeg.getSimdAt(2));

	for (int x = 0; x < 2; ++x)
	{
		for (int y = 0; y < 2; ++y)
		{
			for (int z = 0; z < 2; ++z)
			{
				//Generate the position for the new node.
				const hkVector4& translation = mTransform.getTranslation();
				float posX = (float)translation.getSimdAt(0) + (x ? halfExtentX : -halfExtentX);
				float posY = (float)translation.getSimdAt(1) + (y ? halfExtentX : -halfExtentX);
				float posZ = (float)translation.getSimdAt(2) + (z ? halfExtentX : -halfExtentX);

				position.set(posX, posY, posZ, 0.0f);

				OctNode* node = new OctNode(sceneGraph->getNumCreatedObjects(),
					hkVector4(0.0f, 0.0f, 0.0f, 0.0f), mSceneGraph, this, aabb);

				node->setTranslation(position);

				mChildren.push_back(node);
				mSceneGraph->mOctNodes.push_back(node);
			}
		}
	}

	for (int i = 0; i < 8; ++i)
	{
		mChildren[i]->createChildren(depth, sceneGraph);
	}
}
