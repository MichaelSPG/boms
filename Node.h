#ifndef NODE_H
#define NODE_H

#include "Dx11Renderer.h"
#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>
#include <Common/Base/Types/Geometry/Aabb/hkAabbUtil.h>

class SceneGraph;
class Primitive;
class ShaderManager;

class Node
{
public:
	Node(const hkVector4& position, int id, SceneGraph* sceneGraph)
		: mPosition(0.0f, 0.0f, 0.0f, 0.0f)
		, mID(id)
		, mSceneGraph(sceneGraph)
		, mWireFramePrimitive(nullptr)
	{
		mAABB.setEmpty();
		setPosition(position);
	}
	virtual ~Node();

	//Calls setPosition with position's x, y and z elements.
	inline void setPosition(const hkVector4& position)
	{
		setPosition(position.getSimdAt(0), position.getSimdAt(1), position.getSimdAt(2));
	}

	inline virtual void setPosition(const float x, const float y, const float z)
	{
		hkVector4 translation(mPosition);

		mPosition.set(x, y, z);

		translation.sub3clobberW(mPosition);

		mAABB.m_min.add3clobberW(translation);
		mAABB.m_max.add3clobberW(translation);

		verifyPosition();
	}

	inline const hkVector4& getPosition() const
	{
		return mPosition;
	}


	/**	Creates buffers and prepares the node's AABB for drawing.
	*/
	void createDrawableAabb(Dx11Renderer* dx11Renderer, ShaderManager* shadermanager);

	void drawAABB(Dx11Renderer* dx11Renderer);


protected:
	//Should be extended by subclasses. Called whenever the node is moved.
	virtual void verifyPosition()
	{}

	hkAabb		mAABB;
	Primitive*	mWireFramePrimitive;

	hkVector4	mPosition;
	int			mID;
	SceneGraph*	mSceneGraph;

	XMFLOAT3	mWireframeColor;
};

#endif // NODE_H