#ifndef OCT_NODE_H
#define OCT_NODE_H

#include "Node.h"

#include <vector>
#include <memory>
#include "Primitive.h"
#include "SceneNode.h"
#include "Templates.h"


class SceneGraph;
class SceneNode;

class OctNode : protected Node
{
	friend class SceneGraph;
	friend class SceneNode;

	OctNode(int id, const hkVector4& position, SceneGraph* sceneGraph, OctNode* parent
		, const hkAabb& aabb);

	~OctNode();

public:
	void createChildren(unsigned short depth, SceneGraph* sceneGraph);

	inline const int getID() const
	{
		return mID;
	}
	/*
	inline virtual void setPosition(const float x, const float y, const float z)
	{
		hkVector4 translation(mPosition);

		mPosition.set(x, y, z);

		translation.sub3clobberW(mPosition);

		mAABB.m_min.add3clobberW(translation);
		mAABB.m_max.add3clobberW(translation);
		

		verifyPosition();
	}
	*/

private:
	//Checks if the node should remain as a child of its current OctNode.
	void verifyPosition()
	{

	}

	inline void attachSceneNode(SceneNode* node)
	{
		node->mOctNode = this;
		mSceneNodes.push_back(node);

		mWireframeColor = XMFLOAT3(1.0f, 0.0f, 0.0f);
	}

	inline void detatchSceneNode(SceneNode* node)
	{
		node->mOctNode = nullptr;
		unordered_erase(mSceneNodes, node);

		if (!mSceneNodes.size())
		{
			mWireframeColor = XMFLOAT3(0.25f, 0.25f, 0.25f);
		}
	}


	OctNode*	mParent;
	std::vector<OctNode*>	mChildren;
	std::vector<SceneNode*>	mSceneNodes;

	unsigned short mDepth;
};

#endif // OCT_NODE_H