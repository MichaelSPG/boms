#pragma once

#include <memory>

#include "bsScene.h"

class hkpRigidBody;

class bsMesh;
class bsLine3D;
class bsLight;
class bsSceneNode;
class bsCamera;
class bsText3D;


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
		This will delete the component. If you want to keep the component around, set it
		to null instead.
	*/
	template <typename T>
	inline void detach();


	template <typename T>
	inline T getComponent();


	inline bsSceneNode* getOwner() const
	{
		return mOwner;
	}



//private:
	void syncRigidBodyWithOwner();


	bsSceneNode*	mOwner;

	std::shared_ptr<bsMesh>	mMesh;
	bsLine3D*				mLineRenderer;
	bsLight*				mLight;
	bsCamera*				mCamera;
	bsText3D*				mText3D;

	hkpRigidBody*			mRigidBody;
};

#include "bsEntity.inl"
