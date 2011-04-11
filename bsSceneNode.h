#ifndef BS_SCENE_NODE_H
#define BS_SCENE_NODE_H

#include "bsConfig.h"

#include <vector>
#include <memory>

#include <Common/Base/hkBase.h>
#include <Physics/Dynamics/Phantom/hkpAabbPhantom.h>
#include <Physics/Dynamics/Phantom/hkpCachingShapePhantom.h>
#include <Physics/Collide/Agent/ConvexAgent/BoxBox/hkpBoxBoxAgent.h>
#include <Physics/Collide/Agent/MiscAgent/Phantom/hkpPhantomAgent.h>

#include "bsRenderable.h"
#include "bsTemplates.h"


class bsSceneGraph;

class bsSceneNode
{
	friend class bsSceneGraph;
	friend class Application;//TODO: remove

	bsSceneNode(const hkVector4& position, int id, bsSceneGraph* sceneGraph);
	~bsSceneNode();

public:
	/*	Creates a new scene node which is the child of this scene node.
		The new scene node will inherit this scene node's transform, meaning that moving
		or rotating this node will also move or rotate the child node.
	*/
	bsSceneNode* createChildSceneNode(const hkVector4& localTranslation = hkVector4(0.0f, 0.0f, 0.0f, 0.0f));

	void attachRenderable(const std::shared_ptr<bsRenderable>& renderable);

	void detachRenderable(const std::shared_ptr<bsRenderable>& renderable);

	inline const std::vector<std::shared_ptr<bsRenderable>>& getRenderables() const
	{
		return mRenderables;
	}

	//World space
	//Gets translation derived from all parents, including local.
	const hkVector4& getDerivedTranslation() const;

	//Local space
	//Gets local transformation.
	inline const hkTransform& getTransformation() const
	{
		return mTransform;
	}

	//World space
	//Gets transformation derived from all parents, including local.
	const hkTransform& getDerivedTransformation() const;

	//Local space
	//Gets local translation.
	inline const hkVector4& getTranslation() const
	{
		return mTransform.getTranslation();
	}

	//Calls setPosition with translation's x, y and z elements.
	inline void setTranslation(const hkVector4& translation)
	{
		setTranslation(translation.getSimdAt(0), translation.getSimdAt(1),
			translation.getSimdAt(2));
	}

	void setTranslation(const float x, const float y, const float z);

	//Calls the hkVector4 version of the same function
	inline void translate(const float x, const float y, const float z)
	{
		translate(hkVector4(x, y, z, 0.0f));
	}

	void translate(const hkVector4& translation);

	//Sets local rotation.
	void setRotation(const hkRotation& rotation)
	{
		mTransform.setRotation(rotation);

		updateDerivedTransform();
	}

	//Sets local rotation.
	void setRotation(const hkQuaternion& rotation)
	{
		mTransform.setRotation(rotation);

		updateDerivedTransform();
	}

private:
	//Updates the phantom's shape so that it is identical to the node's AABB.
	void updatePhantomShape();

	//Updates the derived transform so that it includes the most recent version of
	//local transform.
	void updateDerivedTransform() const;

	std::vector<bsSceneNode*> mChildren;
	bsSceneNode*	mParentSceneNode;
	int				mID;

	bsSceneGraph*	mSceneGraph;

	//Every renderable object attached to this scene node.
	std::vector<std::shared_ptr<bsRenderable>>	mRenderables;

	hkTransform				mTransform;
	hkAabb					mAabb;
	hkpCachingShapePhantom*	mPhantom;
};

#endif // BS_SCENE_NODE_H
