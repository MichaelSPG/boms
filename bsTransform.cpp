#include "StdAfx.h"

#include "bsTransform.h"
#include "bsAssert.h"
#include "bsEntity.h"
#include "bsMath.h"

#include <Physics/Dynamics/Entity/hkpRigidBody.h>


bsTransform::bsTransform(bsEntity* ownerEntity)
	: mLocalPosition(XMVectorZero())
	, mLocalRotation(XMQuaternionIdentity())
	, mLocalScale(XMVectorReplicate(1.0f))
	
	, mWorldPosition(XMVectorZero())
	, mWorldRotation(XMQuaternionIdentity())
	, mWorldScale(XMVectorReplicate(1.0f))
	
	, mTransposedWorldTransform(XMMatrixIdentity())
	, mWorldTransform(XMMatrixIdentity())

	, mParentTransform(nullptr)
	, mEntity(ownerEntity)
{
	BS_ASSERT2(((uintptr_t)this) % 16 == 0, "bsTransform must be 16 byte aligned!");
	BS_ASSERT(ownerEntity);
}

void bsTransform::setParentTransform(bsTransform* newParent)
{
	BS_ASSERT2(newParent != this, "Trying to attach a transform to itself");

	mParentTransform = newParent;

	//Update transformations to make the correct with respect to the new parent.
	updateDerivedTransform();

	if (newParent)
	{
		//TODO: Detect cyclic parenting by verifying that all of newParent's parents
		//are not equal to 'this'.

		newParent->mChildren.push_back(this);
	}
}

void bsTransform::setTransformFromRigidBody(const XMVECTOR& position, const XMVECTOR& rotation)
{
	if (mParentTransform)
	{
		mWorldPosition = position;
		mWorldRotation = rotation;

		//Calculate own local position so that own world position is the same as the rigid
		//body's position.
		const XMVECTOR& parentPosition = mParentTransform->getPosition();
		const XMVECTOR deltaPosition = XMVectorSubtract(mWorldPosition, parentPosition);
		mLocalPosition = deltaPosition;

		//TODO: Verify that this works.
		const XMVECTOR& parentRotation = mParentTransform->getRotation();
		mLocalRotation = XMQuaternionMultiply(mWorldRotation,
			XMQuaternionInverse(parentRotation));
	}
	else
	{
		mWorldPosition = mLocalPosition = position;
		mWorldRotation = mLocalRotation = rotation;
	}

	updateDerivedTransform();
}

void bsTransform::setPosition(const XMVECTOR& newPosition)
{
	mWorldPosition = newPosition;

	if (mParentTransform != nullptr)
	{
		//Calculate offset from parent.
		const XMVECTOR& parentPosition = mParentTransform->getPosition();
		mLocalPosition = XMVectorSubtract(mWorldPosition, parentPosition);
	}
	else
	{
		//No parent, local space == world space.
		mLocalPosition = newPosition;
	}

	updateDerivedTransform();

	updateRigidBodyPosition();
}

void bsTransform::setRotation(const XMVECTOR& newRotation)
{
	//mWorldRotation = XMQuaternionInverse(newRotation);
	mWorldRotation = newRotation;

	if (mParentTransform != nullptr)
	{
		//Calculate rotation offset from parent.
		const XMVECTOR& parentRotation = mParentTransform->getRotation();

		mLocalRotation = XMQuaternionMultiply(XMQuaternionInverse(parentRotation), mWorldRotation);
	}
	else
	{
		//No parent, local space == world space.
		mLocalRotation = mWorldRotation;
	}

	updateDerivedTransform();

	updateRigidBodyRotation();
}

void bsTransform::setScale(const XMVECTOR& newScale)
{
	mWorldScale = newScale;

	if (mParentTransform != nullptr)
	{
		//Calculate scale offset from parent.
		const XMVECTOR& parentScale = mParentTransform->getScale();
		mLocalScale = XMVectorDivide(newScale, parentScale);
	}
	else
	{
		//No parent, local space == world space.
		mLocalScale = newScale;
	}

	updateDerivedTransform();
}

void bsTransform::updateDerivedTransform()
{
	//Verify that vectors have not been corrupted.
	BS_ASSERT(!XMVector3IsNaN(mLocalPosition));
	BS_ASSERT(!XMVector3IsInfinite(mLocalPosition));
	BS_ASSERT(!XMVector3IsNaN(mWorldPosition));
	BS_ASSERT(!XMVector3IsInfinite(mWorldPosition));

	BS_ASSERT(!XMQuaternionIsNaN(mLocalRotation));
	BS_ASSERT(!XMQuaternionIsInfinite(mLocalRotation));
	BS_ASSERT(!XMQuaternionIsNaN(mWorldRotation));
	BS_ASSERT(!XMQuaternionIsInfinite(mWorldRotation));

	BS_ASSERT(!XMVector3IsNaN(mLocalScale));
	BS_ASSERT(!XMVector3IsInfinite(mLocalScale));
	BS_ASSERT(!XMVector3IsNaN(mWorldScale));
	BS_ASSERT(!XMVector3IsInfinite(mWorldScale));


	if (mParentTransform != nullptr)
	{
		//In a node hierarchy, include parent transforms in own transform.
		mWorldPosition = XMVectorAdd(mParentTransform->getPosition(), mLocalPosition);
		mWorldRotation = XMQuaternionMultiply(mParentTransform->getRotation(),
			mLocalRotation);
		mWorldScale = XMVectorMultiply(mParentTransform->getScale(), mLocalScale);
	}
	else
	{
		//Not in a hierarchy, own local transform and world transform are identical.
		mWorldPosition = mLocalPosition;
		mWorldRotation = mLocalRotation;
		mWorldScale = mLocalScale;
	}

	//Combine to a full scale * rotation * translation matrix.
	const XMMATRIX positionMat = XMMatrixTranslationFromVector(mWorldPosition);
	const XMMATRIX rotationMat = XMMatrixRotationQuaternion(mWorldRotation);
	const XMMATRIX scaleMat = XMMatrixScalingFromVector(mWorldScale);

	XMMATRIX mat = XMMatrixMultiply(scaleMat, rotationMat);
	mat = XMMatrixMultiply(mat, positionMat);

	//Transpose to save some CPU when sending to the GPU.
	mTransposedWorldTransform = XMMatrixTranspose(mat);

	mWorldTransform = mat;

	for (unsigned int i = 0; i < mChildren.size(); ++i)
	{
		mChildren[i]->updateDerivedTransform();
	}
}

void bsTransform::updateRigidBodyPosition()
{
	hkpRigidBody* rigidBody = mEntity->getRigidBody();
	if (rigidBody != nullptr)
	{
		hkpWorld* world = rigidBody->getWorld();
		if (world != nullptr)
		{
			world->markForWrite();
		}

		rigidBody->setPosition(bsMath::toHK(mWorldPosition));

		if (world != nullptr)
		{
			world->unmarkForWrite();
		}
	}
}

void bsTransform::updateRigidBodyRotation()
{
	hkpRigidBody* rigidBody = mEntity->getRigidBody();
	if (rigidBody != nullptr)
	{
		hkpWorld* world = rigidBody->getWorld();
		if (world != nullptr)
		{
			world->markForWrite();
		}
		hkQuaternion rotation;
		rotation.m_vec = bsMath::toHK(mWorldRotation);
		rigidBody->setRotation(rotation);

		if (world != nullptr)
		{
			world->unmarkForWrite();
		}
	}
}

void bsTransform::updateRigidBodyTransform()
{
	hkpRigidBody* rigidBody = mEntity->getRigidBody();
	if (rigidBody != nullptr)
	{
		hkpWorld* world = rigidBody->getWorld();
		if (world != nullptr)
		{
			world->markForWrite();
		}
		hkQuaternion rotation;
		rotation.m_vec = bsMath::toHK(mWorldRotation);

		rigidBody->setPositionAndRotation(bsMath::toHK(mWorldPosition), rotation);

		if (world != nullptr)
		{
			world->unmarkForWrite();
		}
	}
}
