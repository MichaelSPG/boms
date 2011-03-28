#ifndef NODE_H
#define NODE_H

#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>
#include <Common/Base/Types/Geometry/Aabb/hkAabbUtil.h>

#include "Dx11Renderer.h"

class SceneGraph;
class Primitive;
class ShaderManager;

class Node
{
public:
	Node(const hkVector4& translation, int id, SceneGraph* sceneGraph);

	virtual ~Node();

	//Calls setPosition with translation's x, y and z elements.
	inline void setTranslation(const hkVector4& translation)
	{
		setTranslation(translation.getSimdAt(0), translation.getSimdAt(1),
			translation.getSimdAt(2));
	}

	virtual void setTranslation(const float x, const float y, const float z);

	inline const hkVector4& getTranslation() const
	{
		return mTransform.getTranslation();
	}


	/**	Creates buffers and prepares the node's AABB for drawing.
	*/
	void createDrawableAabb(Dx11Renderer* dx11Renderer, ShaderManager* shadermanager,
		bool octNode = true);

	void drawAABB(Dx11Renderer* dx11Renderer) const;


protected:
	//Should be extended by subclasses. Called whenever the node is moved.
	virtual void verifyTranslation()
	{}

	hkAabb		mAABB;
	Primitive*	mWireFramePrimitive;

	hkTransform	mTransform;
	int			mID;
	SceneGraph*	mSceneGraph;

	XMFLOAT3	mWireframeColor;
};

#endif // NODE_H