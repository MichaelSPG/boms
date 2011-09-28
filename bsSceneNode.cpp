#include "StdAfx.h"

#include "bsSceneNode.h"

#include "bsScene.h"
#include "bsAssert.h"


#pragma warning(push)
//The entity's constructor does not use 'this' for anything but a pointer assignment,
//and 'this' does not have a vtable, so no undefined/unsafe behavior is occuring.
#pragma warning(disable:4355)// warning C4355: 'this' : used in base member initializer list

bsSceneNode::bsSceneNode()
	: mScene(nullptr)
	, mEntity(this)
	, mParentSceneNode(nullptr)
	, mID(~0u)
	, mVisible(true)

	, mLocalPosition(XMVectorZero())
	, mLocalRotation(XMQuaternionIdentity())
	, mLocalScale(XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f))

	, mWorldPosition(XMVectorZero())
	, mWorldRotation(XMQuaternionIdentity())
	, mWorldScale(XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f))

	, mTransposedWorldTransform(XMMatrixIdentity())
	, mWorldTransform(XMMatrixIdentity())

#pragma warning(pop)
{
	BS_ASSERT2(((uintptr_t)this) % 16 == 0, "bsSceneNode must be 16 byte aligned!");
}

bsSceneNode::~bsSceneNode()
{
	//TODO: Figure out if these asserts are useful at all. Probably not unless future
	//features depend on them.
	/*
	BS_ASSERT2(!mChildren.size(), "Destroying a scene node which still has children");
	*/
}

void bsSceneNode::setParentSceneNode(bsSceneNode* newParent)
{
	BS_ASSERT2(newParent != this, "Trying to attach a node to itself");

	mParentSceneNode = newParent;

	if (newParent)
	{
		//Check that if the node is already added to a scene, that it's the same scene as 'this'
		//node is attached to.
		BS_ASSERT2(newParent->mScene == mScene, "Nodes attached to each other must b in the same scene");

		//Update it once to make sure it's synced properly with this node's transform.
		updateDerivedTransform();

		newParent->mChildren.push_back(this);
	}
}

void bsSceneNode::setTransformFromRigidBody()
{
	BS_ASSERT(mEntity.getComponent<hkpRigidBody*>() != nullptr);

	const hkTransform& rbTransform = mEntity.getComponent<hkpRigidBody*>()->getTransform();

	const XMVECTOR rbWorldPosition = bsMath::toXM(rbTransform.getTranslation());
	const XMVECTOR rbWorldRotation = bsMath::toXM(hkQuaternion(rbTransform.getRotation()).m_vec);

	if (mParentSceneNode != nullptr)
	{
		mWorldPosition = rbWorldPosition;
		mWorldRotation = rbWorldRotation;

		//Calculate own local position so that own world position is the same as the rigid
		//body's position.
		const XMVECTOR& parentPosition = mParentSceneNode->getPosition();
		const XMVECTOR deltaPosition = XMVectorSubtract(mWorldPosition, parentPosition);
		mLocalPosition = deltaPosition;
		
		//TODO: Verify that this works.
		const XMVECTOR& parentRotation = mParentSceneNode->getRotation();
		mLocalRotation = XMQuaternionMultiply(mWorldRotation,
			XMQuaternionInverse(parentRotation));
	}
	else
	{
		mWorldPosition = mLocalPosition = rbWorldPosition;
		mWorldRotation = mLocalRotation = rbWorldRotation;
	}

	updateDerivedTransform();
}

void bsSceneNode::addedToScene(bsScene* scene, unsigned int id)
{
	BS_ASSERT(scene != nullptr);
	BS_ASSERT2(mScene == nullptr, "Trying to add a node to a scene, but it is already"
		" added in a scene");

	mID = id;
	mScene = scene;

	hkpRigidBody* rigidBody = mEntity.getComponent<hkpRigidBody*>();
	if (rigidBody != nullptr)
	{
		//Set the rigid body's transform to match this node's transform.
		//Useful for static object which will never otherwise be synced with a scene node.
		hkVector4 position = bsMath::toHK(mWorldPosition);
		hkQuaternion rotation;
		rotation.m_vec = bsMath::toHK(mWorldRotation);

		rigidBody->setPositionAndRotation(position, rotation);
	}
}

void bsSceneNode::setPosition(const XMVECTOR& newPosition)
{
	mWorldPosition = newPosition;

	if (mParentSceneNode != nullptr)
	{
		//Calculate offset from parent.
		const XMVECTOR& parentPosition = mParentSceneNode->getPosition();
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

void bsSceneNode::setRotation(const XMVECTOR& newRotation)
{
	//mWorldRotation = XMQuaternionInverse(newRotation);
	mWorldRotation = newRotation;

	if (mParentSceneNode != nullptr)
	{
		//Calculate rotation offset from parent.
		const XMVECTOR& parentRotation = mParentSceneNode->getRotation();

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

void bsSceneNode::setScale(const XMVECTOR& newScale)
{
	mWorldScale = newScale;

	if (mParentSceneNode != nullptr)
	{
		//Calculate scale offset from parent.
		const XMVECTOR& parentScale = mParentSceneNode->getScale();
		mLocalScale = XMVectorDivide(newScale, parentScale);
	}
	else
	{
		//No parent, local space == world space.
		mLocalScale = newScale;
	}

	updateDerivedTransform();
}

void bsSceneNode::updateDerivedTransform()
{
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


	if (mParentSceneNode != nullptr)
	{
		//In a node hierarchy, include parent transforms in own
		mWorldPosition = XMVectorAdd(mParentSceneNode->getPosition(), mLocalPosition);
		mWorldRotation = XMQuaternionMultiply(mParentSceneNode->getRotation(),
			mLocalRotation);
		mWorldScale = XMVectorMultiply(mParentSceneNode->getScale(), mLocalScale);
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

	std::for_each(std::begin(mChildren), std::end(mChildren),
		[](bsSceneNode* node)
	{
		node->updateDerivedTransform();
	});
}

void bsSceneNode::updateRigidBodyPosition()
{
	hkpRigidBody* rigidBody = mEntity.getComponent<hkpRigidBody*>();
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

void bsSceneNode::updateRigidBodyRotation()
{
	hkpRigidBody* rigidBody = mEntity.getComponent<hkpRigidBody*>();
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

void bsSceneNode::updateRigidBodyTransform()
{
	hkpRigidBody* rigidBody = mEntity.getComponent<hkpRigidBody*>();
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
