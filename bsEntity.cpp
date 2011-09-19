#include "StdAfx.h"

#include "bsEntity.h"
#include "bsSceneNode.h"


bsEntity::bsEntity(bsSceneNode* owner)
	: mOwner(owner)
	, mLineRenderer(nullptr)
	, mLight(nullptr)
	, mCamera(nullptr)
	, mText3D(nullptr)
	, mRigidBody(nullptr)
{

}

bsEntity::~bsEntity()
{
	
}

void bsEntity::syncRigidBodyWithOwner()
{
	mOwner->setTransformFromRigidBody();
}
