#ifndef NODE_H
#define NODE_H

#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>
#include <Common/Base/Types/Geometry/Aabb/hkAabbUtil.h>

#include "bsDx11Renderer.h"

class bsSceneGraph;
class bsPrimitive;
class bsShaderManager;

class bsNode
{
public:
	bsNode(const hkVector4& translation, int id, bsSceneGraph* sceneGraph);

	virtual ~bsNode();

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
	void createDrawableAabb(bsDx11Renderer* dx11Renderer, bsShaderManager* shadermanager,
		bool octNode = true);

	void drawAABB(bsDx11Renderer* dx11Renderer) const;


protected:
	//Should be extended by subclasses. Called whenever the node is moved.
	virtual void verifyTranslation()
	{}

	hkAabb		mAABB;
	bsPrimitive*	mWireFramePrimitive;

	hkTransform	mTransform;
	int			mID;
	bsSceneGraph*	mSceneGraph;

	XMFLOAT3	mWireframeColor;
};

#endif // NODE_H