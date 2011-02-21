#include "SceneNode.h"

#include "SceneGraph.h"
#include "OctNode.h"


SceneNode::SceneNode(const hkVector4& position, int id, SceneGraph* sceneGraph)
	: Node(position, id, sceneGraph)
	, mOctNode(nullptr)
{
	mWireframeColor = XMFLOAT3(1.0f, 0.0f, 1.0f);
}

SceneNode::~SceneNode()
{

}

SceneNode* SceneNode::createChild(const hkVector4& position /*= hkVector4(0.0f, 0.0f, 0.0f, 0.0f)*/)
{
	SceneNode* node = new SceneNode(position, mSceneGraph->getNumCreatedObjects(),
		mSceneGraph);
	mChildren.push_back(node);

	mSceneGraph->mSceneNodes.push_back(node);

	return node;
}

void SceneNode::verifyPosition()
{
	if ((mOctNode->mID != 1) && (mOctNode->mAABB.contains(mAABB)))
	{
		//Still fits in current octNode's AABB, no change necessary.
		return;
	}

	mSceneGraph->placeSceneNode(this, mSceneGraph->mRootNode);
}
