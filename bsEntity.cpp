#include "StdAfx.h"

#include <Common/Base/hkBase.h>
#include <Physics/Dynamics/hkpDynamics.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>

#include "bsEntity.h"
#include "bsText3D.h"
#include "bsAssert.h"
#include "bsLight.h"
#include "bsLineRenderer.h"
#include "bsCamera.h"
#include "bsConvert.h"
#include "bsScene.h"


#pragma warning(push)
//bsTransform's constructor does not use 'this' for anything but a reference assignment,
//and 'this' does not have a vtable, so no undefined/unsafe behavior is occuring.
#pragma warning(disable:4355)// warning C4355: 'this' : used in base member initializer list

bsEntity::bsEntity()
	: mTransform(this)
	, mLineRenderer(nullptr)
	, mLight(nullptr)
	, mCamera(nullptr)
	, mTextRenderer(nullptr)
	, mRigidBody()
	, mScene(nullptr)
	, mSceneID(~0u)
{
	mBoundingSphere.positionAndRadius = XMVectorSet(0.0f, 0.0f, 0.0f, FLT_MAX);

	BS_ASSERT2(((uintptr_t)this) % 16 == 0, "bsEntity must be 16 byte aligned!");
}
#pragma warning(pop)

bsEntity::~bsEntity()
{
	if (mScene)
	{
		mScene->removeEntity(*this);
	}

	//Delete all child entities.
	const std::vector<bsTransform*>& children = mTransform.getChildren();
	for (unsigned int i = 0; i < children.size(); ++i)
	{
		delete &children[i]->getEntity();
	}

	//Delete all owned components.
	delete mLineRenderer;
	delete mLight;
	delete mCamera;
	delete mTextRenderer;
}

void bsEntity::attachRigidBody(hkpRigidBody& rigidBody)
{
	hkpWorld* world = rigidBody.getWorld();
	if (world)
	{
		world->markForWrite();
	}

	BS_ASSERT2(mRigidBody == nullptr, "Trying to attach a rigid body, but a rigid"
		" body is already attached");
	BS_ASSERT2(!rigidBody.hasProperty(BSPK_ENTITY_POINTER), "Trying to attach a"
		" rigid body, but it is still attached to a different entity");

	mRigidBody = &rigidBody;
	mRigidBody->setProperty(BSPK_ENTITY_POINTER, hkpPropertyValue(this));

	//Set the rigid body's position and rotation to the same as this entity's position
	//and rotation.
	const hkVector4 position(bsMath::toHK(mTransform.getPosition()));
	hkQuaternion rotation;
	rotation.m_vec = bsMath::toHK(mTransform.getRotation());

	mRigidBody->setPositionAndRotation(position, rotation);

	if (world)
	{
		world->unmarkForWrite();
	}
}

void bsEntity::attachMesh(const bsSharedMesh& mesh)
{
	BS_ASSERT2(!mMesh, "Trying to attach a mesh, but a mesh is already attached");

	mMesh = mesh;
}

void bsEntity::attachLight(bsLight& light)
{
	BS_ASSERT2(mLight == nullptr, "Trying to attach a light, but a light is already attached");

	mLight = &light;
}

void bsEntity::attachLineRenderer(bsLineRenderer& lineRenderer)
{
	BS_ASSERT2(mLineRenderer == nullptr, "Trying to attach a line renderer, but a line"
		" renderer is already attached");

	mLineRenderer = &lineRenderer;
}

void bsEntity::attachCamera(bsCamera& camera)
{
	BS_ASSERT2(mCamera == nullptr, "Trying to attach a camera, but a camera is already attached");

	mCamera = &camera;
	mCamera->setEntity(this);
}

void bsEntity::attachTextRenderer(bsText3D& textRenderer)
{
	BS_ASSERT2(mTextRenderer == nullptr, "Trying to attach a text renderer, but a text"
		" renderer is already attached");

	mTextRenderer = &textRenderer;
}



void bsEntity::detachRigidBody()
{
	BS_ASSERT2(mRigidBody != nullptr, "Trying to detach a rigid body, but no rigid"
		" body is attached");

	mRigidBody->removeProperty(BSPK_ENTITY_POINTER);
	mRigidBody = nullptr;
}

void bsEntity::detachMesh()
{
	BS_ASSERT2(mMesh, "Trying to detach a mesh, but no mesh is attached");

	mMesh.reset();
}

void bsEntity::detachLight()
{
	BS_ASSERT2(mLight != nullptr, "Trying to detach a light, but no light is attached");

	delete mLight;
	mLight = nullptr;
}

void bsEntity::detachLineRenderer()
{
	BS_ASSERT2(mLineRenderer != nullptr, "Trying to detach a line renderer,"
		" but no line renderer is attached");

	delete mLineRenderer;
	mLineRenderer = nullptr;
}

void bsEntity::detachCamera()
{
	BS_ASSERT2(mCamera != nullptr, "Trying to detach a camera, but no camera is attached");

	delete mCamera;
	mCamera = nullptr;
}

void bsEntity::detachTextRenderer()
{
	BS_ASSERT2(mTextRenderer != nullptr, "Trying to detach a text renderer, but no text"
		" renderer is attached");

	delete mTextRenderer;
	mTextRenderer = nullptr;
}




bsTransform& bsEntity::getTransform()
{
	return mTransform;
}

const bsTransform& bsEntity::getTransform() const
{
	return mTransform;
}

hkpRigidBody* bsEntity::getRigidBody()
{
	return mRigidBody;
}

const hkpRigidBody* bsEntity::getRigidBody() const
{
	return mRigidBody;
}

bsSharedMesh& bsEntity::getMesh()
{
	return mMesh;
}

const bsSharedMesh& bsEntity::getMesh() const
{
	return mMesh;
}

bsLight* bsEntity::getLight()
{
	return mLight;
}

const bsLight* bsEntity::getLight() const
{
	return mLight;
}

bsLineRenderer* bsEntity::getLineRenderer()
{
	return mLineRenderer;
}

const bsLineRenderer* bsEntity::getLineRenderer() const
{
	return mLineRenderer;
}

bsCamera* bsEntity::getCamera()
{
	return mCamera;
}

const bsCamera* bsEntity::getCamera() const
{
	return mCamera;
}

bsText3D* bsEntity::getTextRenderer()
{
	return mTextRenderer;
}

const bsText3D* bsEntity::getTextRenderer() const
{
	return mTextRenderer;
}

void bsEntity::addedToScene(bsScene& scene, unsigned int id)
{
	BS_ASSERT2(mScene == nullptr, "Entity was added to a scene, but it is already in a scene");

	mScene = &scene;
	mSceneID = id;
}

void bsEntity::removedFromScene(bsScene& scene)
{
	BS_ASSERT2(mScene == &scene, "Entity was removed from a scene it was not a part of");

	const std::vector<bsTransform*>& children = mTransform.getChildren();
	for (unsigned int i = 0; i < children.size(); ++i)
	{
		mScene->removeEntity(children[i]->getEntity());
	}

	mScene = nullptr;
}
