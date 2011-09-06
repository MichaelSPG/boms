#pragma once

#include <memory>

#include "bsScene.h"


class bsMesh;
class bsLine3D;
class bsLight;
class bsSceneNode;

class hkpRigidBody;


class bsEntity
{
public:
	explicit bsEntity(bsSceneNode* owner);

	~bsEntity();


	/*	Attaches a component to this entity.
	*/
	template <typename T>
	inline void attach(T component);

	/*	Detaches a component from this entity.
		If an entity of the specified type does not exist, the function call will fail.
	*/
	template <typename T>
	inline void detach();


	template <typename T>
	inline T getComponent() const;


	inline bsSceneNode* getOwner() const
	{
		return mOwner;
	}

private:
	void syncRigidBodyWithOwner();


	bsSceneNode*	mOwner;

	std::shared_ptr<bsMesh>		mMesh;
	bsLine3D*					mLineRenderer;
	bsLight*					mLight;

	hkpRigidBody*				mRigidBody;
};

#include "bsEntity.inl"
