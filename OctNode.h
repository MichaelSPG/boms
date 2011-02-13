#ifndef OCT_NODE_H
#define OCT_NODE_H

#include "Node.h"

#include <vector>
#include <memory>
#include "AABB.h"
#include "Primitive.h"


class SceneGraph;
class SceneNode;

class OctNode : public Node
{
	friend class SceneGraph;
public:
	OctNode(int id, const XMFLOAT3& position, SceneGraph* sceneGraph,
		OctNode* parent, const AABB& aabb);

	~OctNode();

	void createChildren(unsigned short depth, SceneGraph* sceneGraph);

	void attachSceneNode(std::shared_ptr<SceneNode> node);

	inline const int getID() const
	{
		return mID;
	}

	/**	Creates buffers and prepares the node's AABB for drawing.
	*/
	void prepareForDrawing(Dx11Renderer* dx11Renderer, ShaderManager* shadermanager);

	void drawAABB(Dx11Renderer* dx11Renderer);

	

private:
	OctNode*	mParent;
	std::vector<std::shared_ptr<OctNode>>	mChildren;
	std::vector<std::shared_ptr<SceneNode>>	mSceneNodes;

	Primitive*	mWireFramePrimitive;
	AABB*		mAABB;

	float m_color;
};

#endif // OCT_NODE_H