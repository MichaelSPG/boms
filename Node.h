#ifndef NODE_H
#define NODE_H

#include "Dx11Renderer.h"

class SceneGraph;

class Node
{
public:
	Node(const XMFLOAT3& position, int id, SceneGraph* sceneGraph)
		: mPosition(position)
		, mID(id)
		, mSceneGraph(sceneGraph)
	{}
	virtual ~Node()
	{}

	inline void setPosition(const XMFLOAT3& position)
	{
		mPosition = position;
	}

	inline void setPosition(const float x, const float y, const float z)
	{
		mPosition.x = x;
		mPosition.y = y;
		mPosition.z = z;
	}

	inline const XMFLOAT3& getPosition() const
	{
		return mPosition;
	}

protected:
	XMFLOAT3	mPosition;
	int			mID;
	SceneGraph*	mSceneGraph;
};

#endif // NODE_H