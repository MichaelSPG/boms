#pragma once

#include <vector>
#include <memory>

#include <xnamath.h>

#include "bsEntity.h"

class bsScene;
class bsBroadphaseHandle;

class bsSceneNode;


/*	Scene nodes are used to represent a node in 3D space which can have multiple objects
	attached to it.
	For example, a node can be used to represent a car by attaching the car mesh,
	an exhaust particle effect and an engine sound to it.

	It is possible to attach nodes to each other, by calling setParentSceneNode(). This
	results in the node inheriting its parents transform, so if using the above car example,
	each component could be its own node with its own translation offset, scale, or rotation,
	and all three components could share a common parent, resulting in all the nodes being
	moved together whenever the parent node moves.

	After creating a node, add it to a scene to make it active.
*/
__declspec(align(16)) class bsSceneNode
{
public:
	inline void* operator new(size_t);

	inline void operator delete(void* p);


	bsSceneNode();

	~bsSceneNode();

	/*	Returns this node's derived transform components (in this node's world space) .
		The transform components are derived from all the parents of this node.
		If this node is not a child of a different scene node, the transform components
		are the same as the local transform components.
	*/
	inline const XMVECTOR& getPosition() const;

	inline const XMVECTOR& getRotation() const;

	inline const XMVECTOR& getScale() const;

	/*	Returns this node's transposed derived transformation (this node's world space).
		The transformation is derived from node's above this node in the hierarchy.
		If this node is not a child of a different scene node, this transform is the
		same as local transform.
		Note: This is transposed to save resources when uploading it to the GPU.
	*/
	inline const XMMATRIX& getTransposedTransform() const;

	/*	Returns this node's derived transformation (this node's world space).
		The transformation is derived from node's above this node in the hierarchy.
		If this node is not a child of a different scene node, this transform is the
		same as local transform.
	*/
	inline const XMMATRIX& getTransform() const;


	/*	Returns local transform components.
		This is equal to the offset from this node's parent, or equal to this node's
		world transform components if this node does not have a parent.
	*/
	inline const XMVECTOR& getLocalPosition() const;

	inline const XMVECTOR& getLocalRotation() const;

	inline const XMVECTOR& getLocalScale() const;

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

	/*	Sets this node's rotation in world space.
	*/
	void setRotation(const XMVECTOR& rotation);

	/*	Sets this node's local rotation.
	*/
	void setLocalRotation(const XMVECTOR& rotation);

	/*	Sets this node's world scale.
		The w component is ignored.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	void setScale(const XMVECTOR& newScale);

	/*	Sets each component in this node's world scale to the parameter.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	void setScaleUniform(float uniformScale);

	/*	Sets this node's local scale.
		The w component is ignored.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	inline void setLocalScale(const XMVECTOR& newScale);

	/*	Sets each component in this node's local scale to the parameter.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	inline void setLocalScaleUniform(float newUniformScale);

	/*	Scale this node's local scale.
		This multiplies current scale with the parameter.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	inline void scale(const XMVECTOR& scaleToMultiply);

	/*	Scale this node's local scale uniformly.
		This scales the current scale with the parameter.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	inline void scaleUniform(float uniformScale);


	inline bsEntity& getEntity();

	inline const bsEntity& getEntity() const;
	
	/*	Return parent scene node, or null if there is no parent.
	*/
	inline bsSceneNode* getParentSceneNode() const;

	/*	Sets this node's parent node.
		This node will inherit the parent's transform.
		If the parameter is null, the node will no longer have a parent.
	*/
	void setParentSceneNode(bsSceneNode* newParent);

	/*	Returns a read-only vector of this node's children.
	*/
	inline const std::vector<bsSceneNode*>& getChildren() const;



	/*	Internal functions.
	*/

	/*	Called when this scene node is added to a scene.
	*/
	void addedToScene(bsScene* scene, unsigned int id);

	/*	Synchronizes this node with its entity's rigid body's transform.
		Called when a rigid body attached to this node's entity moves.
	*/
	void setTransformFromRigidBody();

	
private:
	void updateRigidBodyPosition();
	void updateRigidBodyRotation();
	void updateRigidBodyTransform();

	/*	Updates world transforms from parent (if there is one) and updates the transposed
		world transform.
	*/
	void updateDerivedTransform();



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

#include "bsSceneNode.inl"
