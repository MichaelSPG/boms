#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include "Node.h"

#include <vector>
#include <memory>

class SceneGraph;

class SceneNode : public Node
{
	friend class SceneGraph;
public:
	SceneNode(const XMFLOAT3& position, int id, SceneGraph* sceneGraph);
	~SceneNode();

	std::shared_ptr<SceneNode> createChild();
	
private:
	std::vector<std::shared_ptr<SceneNode>> mChildren;
};

#endif // SCENE_NODE_H