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

#pragma warning(pop)
{
	BS_ASSERT2((uintptr_t)this % 16 == 0, "bsSceneNode must be 16 byte aligned!");
}

bsSceneNode::~bsSceneNode()
{
	//mPhantom->removeReference();

	//delete mBroadphaseHandle;

	//TODO: Figure out if these asserts are useful at all. Probably not unless future
	//features depend on them.
	/*
	BS_ASSERT2(!mChildren.size(), "Destroying a scene node which still has children");
	*/
}

void bsSceneNode::addChildSceneNode(bsSceneNode* node)
{
	//Check that if the node is already added to a scene, that it's the same scene as 'this'
	//node is attached to.
	BS_ASSERT2(node->mScene == nullptr || node->mScene == mScene,
		"Tried to attach a node from an unrelated scene");

	BS_ASSERT2(node != this, "Trying to attach a node to itself");


	node->mParentSceneNode = this;
	//Update it once to make sure it's synced properly with this node's transform.
	//node->updateDerivedTransform();
	node->updateDerivedTransform();

	mChildren.push_back(node);

	if (node->mScene == nullptr)
	{
		mScene->addSceneNode(node);
	}
}
/*
const hkVector4& bsSceneNode::getDerivedPosition() const
{
	return mPhantom->getTransform().getTranslation();
}

const hkTransform& bsSceneNode::getDerivedTransformation() const
{
	return mPhantom->getTransform();
}
*/
/*
void bsSceneNode::updateDerivedTransform()
{
	//Get derived transform from parent
	if (mParentSceneNode)
	{
		//Multiply own transform with derived.
		hkTransform derivedTransform;
		derivedTransform.setMul(mParentSceneNode->getDerivedTransformation(), mLocalTransform);

		mPhantom->setTransform(derivedTransform);
	}
	else
	{
		//No parents to derive transform from, meaning own (local) transform is
		//already in world space.
		mPhantom->setTransform(mLocalTransform);

		
		mLocalPosition = mWorldPosition;
		mLocalRotation = mWorldRotation;
		mLocalScale = mWorldScale;
	}

	//All children of this node need to have their positions updates too
	for (unsigned int i = 0, count = mChildren.size(); i < count; ++i)
	{
		mChildren[i]->updateDerivedTransform();
	}
}
*/
void bsSceneNode::setLocalPosition(const XMVECTOR& newPosition)
{
	//mLocalTransform.setTranslation(newPosition);
	
	//updateDerivedTransform();
	/*
	hkpRigidBody* rigidBody = mEntity.getComponent<hkpRigidBody*>();
	if (rigidBody != nullptr)
	{
		hkpWorld* world = rigidBody->getWorld();
		if (world != nullptr)
		{
			world->markForWrite();
		}
		
		rigidBody->setPosition(bsMath::toHK(getPosition()));

		//rigidBody->setLocalPosition(getDerivedPosition());
		if (world != nullptr)
		{
			world->unmarkForWrite();
		}
	}
	*/
	if (mParentSceneNode != nullptr)
	{
		
		setPosition(XMVectorAdd(mParentSceneNode->getPosition(), newPosition));
	}
	else
	{
		setPosition(newPosition);
	}
	
	//mLocalPosition = bsMath::toXM(newPosition);
	updateDerivedTransform();
}

void bsSceneNode::translate(const XMVECTOR& translation)
{
	XMVECTOR newPosition = XMVectorAdd(mLocalPosition, translation);
	setLocalPosition(newPosition);
	/*
	hkVector4 newPosition(mLocalTransform.getTranslation());
	newPosition.add(translation);
	setLocalPosition(newPosition);
	*/
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

	/*
	mLocalTransform = mEntity.getComponent<hkpRigidBody*>()->getTransform();

	updateDerivedTransform();

	XMFLOAT4 v;
	v = bsMath::toXM4(mLocalTransform.getTranslation());
	mLocalPosition = XMLoadFloat4(&v);
	
	v = bsMath::toXM(hkQuaternion(mLocalTransform.getRotation()));
	mLocalRotation = XMLoadFloat4(&v);
	*/
	updateDerivedTransform();
}

void bsSceneNode::setLocalRotation(const hkRotation& rotation)
{
	setLocalRotation(hkQuaternion(rotation));
	/*
	mLocalTransform.setLocalRotation(rotation);

	updateDerivedTransform();

	hkpRigidBody* rigidBody = mEntity.getComponent<hkpRigidBody*>();
	if (rigidBody != nullptr)
	{
		rigidBody->setLocalRotation(hkQuaternion(getDerivedTransformation().getRotation()));
	}
	*/
}

void bsSceneNode::setLocalRotation(const hkQuaternion& rotation)
{
	mLocalRotation = bsMath::toXM(rotation);
	//TODO: Figure out how to skip this inversion.
	mLocalRotation = XMQuaternionInverse(mLocalRotation);

	updateDerivedTransform();


	//mLocalTransform.setRotation(rotation);

	//updateDerivedTransform();

	hkpRigidBody* rigidBody = mEntity.getComponent<hkpRigidBody*>();
	if (rigidBody != nullptr)
	{
		hkpWorld* world = rigidBody->getWorld();
		if (world != nullptr)
		{
			world->markForWrite();
		}
		hkQuaternion rot;
		rot.m_vec = bsMath::toHK(mWorldRotation);
		rigidBody->setRotation(rot);

		//rigidBody->setLocalRotation(hkQuaternion(getDerivedTransformation().getRotation()));
		if (world != nullptr)
		{
			world->unmarkForWrite();
		}
	}
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
		const XMVECTOR& parentPosition = mParentSceneNode->getPosition();
		mLocalPosition = XMVectorSubtract(mWorldPosition, parentPosition);
	}
	else
	{
		mLocalPosition = newPosition;
	}
}

void bsSceneNode::updateDerivedTransform()
{
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
}
