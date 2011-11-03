#include "StdAfx.h"

#include <algorithm>

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
#include "bsMesh.h"


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
	mBoundingSphere.positionAndRadius = XMVectorSet(0.0f, 0.0f, 0.0f, FLT_MIN);

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
	mMesh->attachedToEntity(*this);

	updateBoundingSphere(mesh->getBoundingSphere());
}

void bsEntity::attachLight(bsLight& light)
{
	BS_ASSERT2(mLight == nullptr, "Trying to attach a light, but a light is already attached");

	mLight = &light;

	updateBoundingSphere(light.getBoundingSphere());
}

void bsEntity::attachLineRenderer(bsLineRenderer& lineRenderer)
{
	BS_ASSERT2(mLineRenderer == nullptr, "Trying to attach a line renderer, but a line"
		" renderer is already attached");

	mLineRenderer = &lineRenderer;

	mLineRenderer->attachedToEntity(*this);

	updateBoundingSphere(lineRenderer.getBoundingSphere());
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

	updateBoundingSphere(textRenderer.getBoundingSphere());
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

	calculateLocalBoundingSphere();
}

void bsEntity::detachLight()
{
	BS_ASSERT2(mLight != nullptr, "Trying to detach a light, but no light is attached");

	delete mLight;
	mLight = nullptr;

	calculateLocalBoundingSphere();
}

void bsEntity::detachLineRenderer()
{
	BS_ASSERT2(mLineRenderer != nullptr, "Trying to detach a line renderer,"
		" but no line renderer is attached");

	delete mLineRenderer;
	mLineRenderer = nullptr;

	calculateLocalBoundingSphere();
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

	calculateLocalBoundingSphere();
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

const bsScene* bsEntity::getScene() const
{
	return mScene;
}

void bsEntity::updateBoundingSphere(const bsCollision::Sphere& newSphereToInclude)
{
	//If the new sphere is completely contained by the current bounding sphere, it does
	//not need to be recalculated.
	if (bsCollision::intersectSphereSphere(mBoundingSphere, mTransform.getPosition(),
		newSphereToInclude, mTransform.getPosition())
		!= bsCollision::INSIDE)
	{
		mBoundingSphere	= bsCollision::mergeSpheres(mBoundingSphere, newSphereToInclude);
	}
}

void bsEntity::calculateLocalBoundingSphere()
{
	bsCollision::Sphere newBoundingSphere;
	newBoundingSphere.positionAndRadius = XMVectorSet(0.0f, 0.0f, 0.0f, FLT_MIN);

	if (mMesh)
	{
		newBoundingSphere = bsCollision::mergeSpheres(newBoundingSphere,
			mMesh->getBoundingSphere());
	}
	if (mLight)
	{
		newBoundingSphere = bsCollision::mergeSpheres(newBoundingSphere,
			mLight->getBoundingSphere());
	}
	if (mLineRenderer)
	{
		newBoundingSphere = bsCollision::mergeSpheres(newBoundingSphere,
			mLineRenderer->getBoundingSphere());
	}
	if (mTextRenderer)
	{
		newBoundingSphere = bsCollision::mergeSpheres(newBoundingSphere,
			mTextRenderer->getBoundingSphere());
	}

	mBoundingSphere = newBoundingSphere;
}

bsCollision::Sphere bsEntity::getBoundingSphere() const
{
	bsCollision::Sphere scaledBoundingSphere = mBoundingSphere;

	//Find the largest scaled axis. Only the single largest axis is interesting when
	//dealing with spheres.
	XMFLOAT4A scale;
	XMStoreFloat4A(&scale, mTransform.getScale());
	const float maxScale = std::max(std::max(scale.x, scale.y), scale.z);

	//The bounding sphere's center needs to be modified by the current rotation and scale.
	scaledBoundingSphere.positionAndRadius =
		XMVector3Rotate(scaledBoundingSphere.positionAndRadius, mTransform.getRotation());
	scaledBoundingSphere.positionAndRadius =
		XMVectorScale(scaledBoundingSphere.positionAndRadius, maxScale);

	//Scale the bounding sphere's radius by the largest scaled axis.
	//This ensures that the bounding sphere will always contain scaled objects.
	scaledBoundingSphere.setRadius(mBoundingSphere.getRadius() * maxScale);

	return scaledBoundingSphere;
}
