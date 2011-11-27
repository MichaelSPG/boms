#pragma once

#include <xnamath.h>

class bsEntity;


/*	A transform contains contains position, rotation and scale in both local and world space.

	Transforms can also represent child/parent hierarchies, where a child's local transform
	is relative to its parent.

	Every entity has one transform.
*/
__declspec(align(16)) class bsTransform
{
public:
	inline void* operator new(size_t);

	inline void operator delete(void* p);


	/*	Initializes this transform to identity.
	*/
	bsTransform(bsEntity* ownerEntity);



	/*	Returns this transform's derived transform components (in this transform's world space) .
		The transform components are derived from all the parents of this transform.
		If this transform is not a child of a different transform, the transform components
		are the same as the local transform components.
	*/
	inline const XMVECTOR& getPosition() const;

	inline const XMVECTOR& getRotation() const;

	inline const XMVECTOR& getScale() const;

	/*	Returns this transform's transposed derived transformation (this transform's world space).
		The transformation is derived from transform's above this transform in the hierarchy.
		If this transform is not a child of a different transform, this transform is the
		same as local transform.
		Note: This is transposed to save resources when uploading it to the GPU.
	*/
	inline const XMMATRIX& getTransposedTransform() const;

	/*	Returns this transform's derived transformation (this transform's world space).
		The transformation is derived from transform's above this transform in the hierarchy.
		If this transform is not a child of a different transform, this transform is the
		same as local transform.
	*/
	inline const XMMATRIX& getTransform() const;


	/*	Returns local transform components.
		This is equal to the offset from this transform's parent, or equal to this transform's
		world transform components if this transform does not have a parent.
	*/
	inline const XMVECTOR& getLocalPosition() const;

	inline const XMVECTOR& getLocalRotation() const;

	inline const XMVECTOR& getLocalScale() const;

	/*	Set this transform's world position.
	*/
	void setPosition(const XMVECTOR& newPosition);

	/*	Sets this transform's local position.
	*/
	void setLocalPosition(const XMVECTOR& newPosition);

	/*	Translates this transform's local translation.
		This adds the input to the current position.
	*/
	void translate(const XMVECTOR& translation);

	/*	Sets this transform's rotation in world space.
	*/
	void setRotation(const XMVECTOR& newRotation);

	/*	Sets this transform's local rotation.
	*/
	void setLocalRotation(const XMVECTOR& rotation);

	/*	Sets this transform's world scale.
		The w component is ignored.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	void setScale(const XMVECTOR& newScale);

	/*	Sets each component in this transform's world scale to the parameter.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	void setScaleUniform(float uniformScale);

	/*	Sets this transform's local scale.
		The w component is ignored.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	inline void setLocalScale(const XMVECTOR& newScale);

	/*	Sets each component in this transform's local scale to the parameter.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	inline void setLocalScaleUniform(float newUniformScale);

	/*	Scale this transform's local scale.
		This multiplies current scale with the parameter.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	inline void scale(const XMVECTOR& scaleToMultiply);

	/*	Scale this transform's local scale uniformly.
		This scales the current scale with the parameter.
		Note: This will NOT scale collision shapes, only graphical components.
	*/
	inline void scaleUniform(float uniformScale);


	inline bsEntity& getEntity();

	inline const bsEntity& getEntity() const;
	
	/*	Return parent transform, or null if there is no parent.
	*/
	inline bsTransform* getParentTransform() const;

	/*	Sets this transform's parent transform.
		This transform will inherit the parent's transform.
		If the parameter is null, the transform will no longer have a parent.
	*/
	void setParentTransform(bsTransform* newParent);

	/*	Returns a read-only vector of this transform's children.
	*/
	inline const std::vector<bsTransform*>& getChildren() const;

	/*	Removes the child from this transform's list of children.
		The child will not have a parent when this function returns.
	*/
	void unparentChild(bsTransform& childToRemove);


	/*	Internal functions.
	*/

	/*	Synchronizes this transform with its entity's rigid body's transform.
		Called when a rigid body attached to this transform's entity moves.
		Unlike setPosition and setRotation, this does not update the rigid body's
		transform.
	*/
	void setTransformFromRigidBody(const XMVECTOR& position, const XMVECTOR& rotation);

	
private:
	/*	These functions are called after position/rotation has been changed.
		Used to keep en entity's rigid body's transform in sync with the entity's transform.
	*/
	void updateRigidBodyPosition();
	void updateRigidBodyRotation();
	void updateRigidBodyTransform();

	/*	Updates world transforms from parent (if there is one) and updates the transposed
		world transform.
		It's assumed that local position/rotation/scale is set before calling this function.
	*/
	void updateDerivedTransform();

	//Non-copyable
	bsTransform(const bsTransform&);
	bsTransform& operator=(const bsTransform&);

	
	__declspec(align(16))XMVECTOR	mLocalPosition;
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


	/*	This transform's parent, or null if there is no parent.
	*/
	bsTransform* mParentTransform;

	std::vector<bsTransform*> mChildren;

	bsEntity* const	mEntity;
};

#include "bsTransform.inl"
