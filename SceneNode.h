#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include "Node.h"

#include <vector>
#include <memory>


class SceneGraph;
class OctNode;

class SceneNode : public Node
{
	friend class SceneGraph;
	friend class OctNode;
	friend class Application;//TODO: remove

	SceneNode(const hkVector4& position, int id, SceneGraph* sceneGraph);
	~SceneNode();

public:
	

	SceneNode* createChild(const hkVector4& position = hkVector4(0.0f, 0.0f, 0.0f, 0.0f));
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
	inline void verifyPosition();

	std::vector<SceneNode*> mChildren;
	OctNode*	mOctNode;
};

#endif // SCENE_NODE_H