#include "SceneNode.h"

#include "SceneGraph.h"


SceneNode::SceneNode(const XMFLOAT3& position, int id, SceneGraph* sceneGraph)
	: Node(position, id, sceneGraph)
{

}

SceneNode::~SceneNode()
{

}

std::shared_ptr<SceneNode> SceneNode::createChild()
{
	std::shared_ptr<SceneNode> node = std::make_shared<SceneNode>(XMFLOAT3(),
		mSceneGraph->getNumCreatedObjects(), mSceneGraph);
	mChildren.push_back(node);

	return node;
}
