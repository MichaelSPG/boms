#pragma once

#include <vector>
#include <memory>

#include <xnamath.h>

#include "bsEntity.h"

class bsScene;
class bsBroadphaseHandle;

class bsSceneNode;

void* allocateNode();
void deallocateNode(void* p);


/*	Scene nodes are used to represent a node in 3D space which can have multiple objects
	attached to it.
	For example, a node can be used to represent a car by attaching the car mesh,
	an exhaust particle effect and an engine sound to it.

	After creating a node, add it to a scene to make it active.
*/
__declspec(align(16)) class bsSceneNode
{
	friend class bsScene;
public:
	void* operator new(size_t)
	{
		return allocateNode();
	}

	void operator delete(void* p)
	{
		deallocateNode(p);
	}


	bsSceneNode();

	~bsSceneNode();

	/*	Attaches a node to this node.
		The node will inherit this node's transform, meaning that moving, rotating  or
		scaling this node will also move, rotate or scale the child node.

		This will add the node to the same scene as this node if this node has been added
		to a scene.
	*/
	void addChildSceneNode(bsSceneNode* node);


	/*	Returns this node's derived transform components (in this node's world space) .
		The transform components are derived from all the parents of this node.
		If this node is not a child of a different scene node, the transform components
		are the same as the local transform components.
	*/
	inline const XMVECTOR& getPosition() const
	{
		return mWorldPosition;
	}

	inline const XMVECTOR& getRotation() const
	{
		return mWorldRotation;
	}

	inline const XMVECTOR& getScale() const
	{
		return mLocalScale;
	}

	/*	Returns this node's transposed derived transformation (this node's world space).
		The transformation is derived from node's above this node in the hierarchy.
		If this node is not a child of a different scene node, this transform is the
		same as local transform.
		Note: This is transposed to save resources when uploading it to the GPU.
	*/
	inline const XMMATRIX& getTransposedTransform() const
	{
		return mTransposedWorldTransform;
	}

	/*	Returns this node's derived transformation (this node's world space).
		The transformation is derived from node's above this node in the hierarchy.
		If this node is not a child of a different scene node, this transform is the
		same as local transform.
	*/
	inline const XMMATRIX& getTransform() const
	{
		return mWorldTransform;
	}


	/*	Returns local transform components.
		This is equal to the offset from this node's parent, or equal to this node's
		world transform components if this node does not have a parent.
	*/
	inline const XMVECTOR& getLocalPosition() const
	{
		return mLocalPosition;
	}

	inline const XMVECTOR& getLocalRotation() const
	{
		return mLocalRotation;
	}

	inline const XMVECTOR& getLocalScale() const
	{
		return mLocalScale;
	}

	/*	Set this node's world position.
	*/
	void setPosition(const XMVECTOR& newPosition);

	/*	Sets this node's local position.
	*/
	void setLocalPosition(const XMVECTOR& newPosition);

	/*	Translates this node's local translation.
		This adds the input to the current position.
	*/
	void translate(const XMVECTOR& translation);

	/*	Sets this node's local rotation.
	*/
	void setLocalRotation(const hkRotation& rotation);

	/*	Sets this node's local rotation.
	*/
	void setLocalRotation(const hkQuaternion& rotation);

	//TODO: Remove the other two overloads.
	inline void setLocalRotation(const XMVECTOR& rotation)
	{
		hkQuaternion q;
		q.m_vec = bsMath::toHK(rotation);
		setLocalRotation(q);
	}

	/*	Sets this node's local scale.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	inline void setLocalScale(const XMVECTOR& newScale)
	{
		mLocalScale = newScale;

		updateDerivedTransform();
	}

	/*	Sets this node's local scale's xyz components to the parameter.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	inline void setLocalScaleUniform(float newUniformScale)
	{
		setLocalScale(XMVectorSet(newUniformScale, newUniformScale, newUniformScale, 1.0f));
	}

	/*	Scale this node's local scale.
		This multiplies own scale with the parameter.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	inline void scale(const XMVECTOR& scaleToMultiply)
	{
		setLocalScale(XMVectorMultiply(mLocalScale, scaleToMultiply));
	}

	/*	Scale this node's local scale uniformly.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	inline void scaleUniform(float uniformScale)
	{
		setLocalScale(XMVectorScale(mLocalScale, uniformScale));
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

	inline bsEntity& getEntity()
	{
		return mEntity;
	}

	inline const bsEntity& getEntity() const
	{
		return mEntity;
	}
	


	/*	Internal functions.
	*/

	/*	Called when this scene node is added to a scene.
	*/
	void addedToScene(bsScene* scene, unsigned int id);

	/*	Synchronizes this node with its entity's rigid body's transform.
		Called when a rigid body attached to this node's entity moves.
	*/
	void setTransformFromRigidBody();

	/*	Updates world transforms from parent (if there is one) and updates the transposed
		world transform.
	*/
	void updateDerivedTransform();


private:
	std::vector<bsSceneNode*> mChildren;
	/*	This node's parent, or null if there is no parent.
	*/
	bsSceneNode*	mParentSceneNode;
	unsigned int	mID;
	bool			mVisible;

	/*	The scene in which this node exists, or null if it is not in a scene.
	*/
	bsScene*	mScene;

	bsEntity	mEntity;
	
	XMVECTOR	mLocalPosition;
	XMVECTOR	mLocalRotation;
	XMVECTOR	mLocalScale;

	/*	Transforms in world space, i.e. derived from all parents, or equal to local
		transform if there is no parent.
	*/
	XMVECTOR	mWorldPosition;
	XMVECTOR	mWorldRotation;
	XMVECTOR	mWorldScale;

	/*	Stored as transposed because that is what is needed for the GPU.		
	*/
	XMMATRIX	mTransposedWorldTransform;
	
	/*	Untransposed of the above.	
	*/
	XMMATRIX	mWorldTransform;
};



inline void* allocateNode()
{
	return _aligned_malloc(sizeof(bsSceneNode), 16);
}

inline void deallocateNode(void* ptr)
{
	_aligned_free(ptr);
}
