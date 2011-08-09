#pragma once

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
class bsBroadphaseHandle;


/*	Scene nodes are used to represent a node in 3D space which can have multiple objects
	attached to it.
	For example, a node can be used to represent a car by attaching the car mesh,
	an exhaust particle effect and an engine sound to it.

	Use bsSceneGraph to create scene nodes, or an already created scene node if you want
	to create a hierarchy of nodes.
*/
class bsSceneNode
{
	friend class bsSceneGraph;

	bsSceneNode(const hkVector4& position, int id, bsSceneGraph* sceneGraph);

	~bsSceneNode();

public:
	/*	Creates a new scene node which is the child of this scene node.
		The new scene node will inherit this scene node's transform, meaning that moving
		or rotating this node will also move or rotate the child node.
	*/
	bsSceneNode* createChildSceneNode(const hkVector4& localTranslation =
		hkVector4(0.0f, 0.0f, 0.0f, 0.0f));
	
	/*	Attaches a renderable object to this scene node.
		This allows the object to be rendered with this node's transformations.
	*/
	void attachRenderable(const std::shared_ptr<bsRenderable>& renderable);

	/*	Detaches a renderable, meaning it will no longer be drawn with this node's
		transformation.
	*/
	void detachRenderable(const std::shared_ptr<bsRenderable>& renderable);

	/*	Returns all renderables attached to this node.
	*/
	inline const std::vector<std::shared_ptr<bsRenderable>>& getRenderables() const
	{
		return mRenderables;
	}

	/*	Returns this node's derived translation (this node's world space).
		The translation is derived from node's above this node in the hierarchy.
		If this node is not a child of a different scene node, this translation is the
		same as local translation.
	*/
	const hkVector4& getDerivedPosition() const;

	/*	Returns this node's local transformation.
	*/
	inline const hkTransform& getTransformation() const
	{
		return mLocalTransform;
	}

	/*	Returns this node's derived transformation (this node's world space).
		The transformation is derived from node's above this node in the hierarchy.
		If this node is not a child of a different scene node, this transformation is the
		same as local translation.
	*/
	const hkTransform& getDerivedTransformation() const;

	/*	Returns this node's local translation (local space).
	*/
	inline const hkVector4& getPosition() const
	{
		return mLocalTransform.getTranslation();
	}

	/*	Sets this node's local translation.
	*/
	void setPosition(const hkVector4& newPosition);

	/*	Translates this node's local translation.
	*/
	void translate(const hkVector4& translation);

	/*	Sets this node's local rotation.
	*/
	inline void setRotation(const hkRotation& rotation)
	{
		mLocalTransform.setRotation(rotation);

		updateDerivedTransform();
	}

	/*	Sets this node's local rotation.
	*/
	inline void setRotation(const hkQuaternion& rotation)
	{
		mLocalTransform.setRotation(rotation);

		updateDerivedTransform();
	}

	/*	Sets whether this node should be visible or not.
		If it is not visible, any renderable objects it contain will not be rendered.
		Default to true.
	*/
	inline void setVisible(bool visible)
	{
		mVisible = visible;
	}

	inline bool isVisible() const
	{
		return mVisible;
	}

private:
	/*	Updates the phantom's shape so that it is identical to the node's AABB.
		Called after a renderable has been detached or attached to keep visibility
		detection correct.
	*/
	void updatePhantomShape();

	/*	Updates the derived transform so that it includes the most recent version of
		local transform.
		Called by all functions which modify local transform.
	*/
	void updateDerivedTransform() const;

	std::vector<bsSceneNode*> mChildren;
	bsSceneNode*	mParentSceneNode;
	int				mID;
	bool			mVisible;

	bsSceneGraph*	mSceneGraph;

	//Every renderable object attached to this scene node.
	std::vector<std::shared_ptr<bsRenderable>>	mRenderables;

	hkTransform				mLocalTransform;
	hkAabb					mAabb;
	hkpCachingShapePhantom*	mPhantom;
	bsBroadphaseHandle*		mBroadphaseHandle;
};
