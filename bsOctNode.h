#ifndef BS_OCT_NODE_H
#define BS_OCT_NODE_H

#include "bsNode.h"

#include <vector>
#include <memory>
#include "bsPrimitive.h"
#include "bsSceneNode.h"
#include "bsTemplates.h"


class bsSceneGraph;
class bsSceneNode;

class bsOctNode : protected bsNode
{
	friend class bsSceneGraph;
	friend class bsSceneNode;
	friend class Application;

	bsOctNode(int id, const hkVector4& position, bsSceneGraph* sceneGraph, bsOctNode* parent
		, const hkAabb& aabb);

	~bsOctNode();

public:
	void createChildren(unsigned short depth, bsSceneGraph* sceneGraph);

	inline const int getID() const
	{
		return mID;
	}

	//World space.
	inline const hkTransform& getTransformation()
	{
		//mTransform is always in world space for oct nodes, so no need to update from
		//parents.
		return mTransform;
	}

private:
	//Checks if the node should remain as a child of its current bsOctNode.
	void verifyPosition()
	{

	}

	inline void attachSceneNode(bsSceneNode* node)
	{
		node->mOctNode = this;
		mSceneNodes.push_back(node);

		mWireframeColor = XMFLOAT3(1.0f, 0.0f, 0.0f);
	}

	inline void detatchSceneNode(bsSceneNode* node)
	{
		node->mOctNode = nullptr;
		unordered_erase(mSceneNodes, node);

		if (!mSceneNodes.size())
		{
			mWireframeColor = XMFLOAT3(0.25f, 0.25f, 0.25f);
		}
	}


	bsOctNode*	mParent;
	std::vector<bsOctNode*>	mChildren;
	std::vector<bsSceneNode*>	mSceneNodes;

	unsigned short mDepth;
};

#endif // BS_OCT_NODE_H
