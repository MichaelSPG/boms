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
	friend class Application;

	OctNode(int id, const hkVector4& position, SceneGraph* sceneGraph, OctNode* parent
		, const hkAabb& aabb);

	~OctNode();

public:
	void createChildren(unsigned short depth, SceneGraph* sceneGraph);

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