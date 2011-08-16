#include "bsSceneNode.h"

#include <Physics/Dynamics/hkpDynamics.h>
#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>

#include "bsSceneGraph.h"
#include "bsResourceManager.h"
#include "bsHavokManager.h"
#include "bsMesh.h"
#include "bsLine3D.h"
#include "bsAssert.h"
#include "bsBroadphaseHandle.h"


bsSceneNode::bsSceneNode(const hkVector4& localTranslation, int id, bsSceneGraph* sceneGraph)
	: mSceneGraph(sceneGraph)
	, mParentSceneNode(nullptr)
	, mID(id)
	, mVisible(true)
	, mPhantom(nullptr)
{
	mLocalTransform.getTranslation().setXYZ(localTranslation);
	mLocalTransform.getRotation().setIdentity();

	mAabb.setEmpty();

	hkpBoxShape* shape = new hkpBoxShape(hkVector4(0.1f, 0.1f, 0.1f));
	mPhantom = new hkpCachingShapePhantom(shape, mLocalTransform);
	
	//Set user data to point to this scene node, making it possible to get this scene node
	//from other phantoms' lists of overlapping phantoms.
	mPhantom->setUserData(reinterpret_cast<hkUlong>(this));
	mSceneGraph->mGraphicsWorld->addPhantom(mPhantom);



	//TODO: Fix this stuff, move addUserObjects elsewhere
	
	mBroadphaseHandle = new bsBroadphaseHandle(this);
	
	hkpTreeBroadPhase* bp = static_cast<hkpTreeBroadPhase*>(mSceneGraph->mGraphicsWorld->getBroadPhase());
	hkpBroadPhaseHandle* handles[1] =
	{
		mBroadphaseHandle
	};
	hkAabb aabbs[1] =
	{
		mAabb
	};
	mPhantom->calcAabb(aabbs[0]);
	bp->addUserObjects(1, handles, aabbs);
}

bsSceneNode::~bsSceneNode()
{
	mPhantom->removeReference();

	delete mBroadphaseHandle;

	//TODO: Figure out if these asserts are useful at all. Probably not unless future
	//features depend on them.
	/*
	BS_ASSERT2(!mRenderables.size(), "Destroying a scene node which still has renderables attached");
	BS_ASSERT2(!mChildren.size(), "Destroying a scene node which still has children");
	*/
}

bsSceneNode* bsSceneNode::createChildSceneNode(const hkVector4& position /*= hkVector4(0.0f, 0.0f, 0.0f, 0.0f)*/)
{
	bsSceneNode* node = new bsSceneNode(position, mSceneGraph->getNewId(),
		mSceneGraph);
	node->mParentSceneNode = this;

	//Update it once to make sure it's synced properly with this node.
	node->updateDerivedTransform();

	mChildren.push_back(node);

	mSceneGraph->mSceneNodes.push_back(node);

	return node;
}

const hkVector4& bsSceneNode::getDerivedPosition() const
{
	return mPhantom->getTransform().getTranslation();
}

const hkTransform& bsSceneNode::getDerivedTransformation() const
{
	return mPhantom->getTransform();
}

void bsSceneNode::updateDerivedTransform() const
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
	}

	//All children of this node need to have their positions updates too
	for (unsigned int i = 0, count = mChildren.size(); i < count; ++i)
	{
		mChildren[i]->updateDerivedTransform();
	}
}

void bsSceneNode::setPosition(const hkVector4& newPosition)
{
	hkVector4 translation(mLocalTransform.getTranslation());
	translation.subXYZ(newPosition);

	mLocalTransform.setTranslation(newPosition);
	//mLocalTransform.getTranslation().set(x, y, z, 0.0f);

	//Get translation
	translation.subXYZ(mLocalTransform.getTranslation());
	
	updateDerivedTransform();
}

void bsSceneNode::translate(const hkVector4& translation)
{
	mLocalTransform.getTranslation().addXYZ(translation);
	
	updateDerivedTransform();
}

void bsSceneNode::attachRenderable(const std::shared_ptr<bsRenderable>& renderable)
{
	//TODO: Consider removing this if implementing async file loading
	BS_ASSERT2(renderable->hasFinishedLoading(), "Trying to attach a renderable to a scene"
		" node which is not ready to be rendered");

	//Extend this node's AABB to include that of the new renderable.
	mAabb.includeAabb(renderable->getAabb());

	mRenderables.push_back(renderable);

	updatePhantomShape();
}

void bsSceneNode::detachRenderable(const std::shared_ptr<bsRenderable>& renderable)
{
	for (unsigned int i = 0, count = mRenderables.size(); i < count; ++i)
	{
		if (mRenderables[i] == renderable)
		{
			bs::unordered_erase(mRenderables, mRenderables[i]);
			
			break;
		}
	}

	if (mRenderables.empty())
	{
		//Can't reconstruct the phantom with an empty AABB, so make it really small instead
		mAabb.m_min.setAll(-FLT_MIN);
		mAabb.m_max.setAll(FLT_MIN);
		updatePhantomShape();

		return;
	}

	//Rebuild the AABB.
	mAabb.setEmpty();
	for (unsigned int i = 0, count = mRenderables.size(); i < count; ++i)
	{
		mAabb.includeAabb(mRenderables[i]->getAabb());
	}

	updatePhantomShape();
}

void bsSceneNode::updatePhantomShape()
{
	//Create a new box shape with half extents equal to this node's AABB's half extents.
	hkVector4 halfExtents;
	mAabb.getHalfExtents(halfExtents);
	hkpBoxShape* shape = new hkpBoxShape(halfExtents);
	mPhantom->setShape(shape);
	shape->removeReference();
}
