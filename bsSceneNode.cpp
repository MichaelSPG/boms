#include "bsSceneNode.h"

#include <cassert>

#include "bsSceneGraph.h"
#include "bsResourceManager.h"
#include "bsHavokManager.h"
#include "bsMesh.h"
#include "bsGeometryUtils.h"
#include "bsLine3D.h"
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>


bsSceneNode::bsSceneNode(const hkVector4& localTranslation, int id, bsSceneGraph* sceneGraph)
	: mSceneGraph(sceneGraph)
	, mParentSceneNode(nullptr)
	, mID(id)
	, mPhantom(nullptr)
{
	mTransform.getTranslation().setXYZ(localTranslation);
	mTransform.getRotation().setIdentity();

	mAabb.setEmpty();

	hkpBoxShape* shape = new hkpBoxShape(hkVector4(0.1f, 0.1f, 0.1f));
	mPhantom = new hkpCachingShapePhantom(shape, mTransform);
	
	//Set user data to point to this scene node, making it possible to get this scene node
	//from other phantoms' lists of overlapping phantoms.
	mPhantom->setUserData(reinterpret_cast<hkUlong>(this));
	mSceneGraph->mHavokManager->getGraphicsWorld()->addPhantom(mPhantom);
}

bsSceneNode::~bsSceneNode()
{
	mPhantom->removeReference();

	assert(!mChildren.size());
	assert(!mRenderables.size());
	assert(!mParentSceneNode);
}

bsSceneNode* bsSceneNode::createChildSceneNode(const hkVector4& position /*= hkVector4(0.0f, 0.0f, 0.0f, 0.0f)*/)
{
	bsSceneNode* node = new bsSceneNode(position, mSceneGraph->getNewId(),
		mSceneGraph);
	node->mParentSceneNode = this;

	mChildren.push_back(node);

	mSceneGraph->mSceneNodes.push_back(node);

	return node;
}

const hkVector4& bsSceneNode::getDerivedTranslation() const
{
	return mPhantom->getTransform().getTranslation();
}

const hkTransform& bsSceneNode::getDerivedTransformation() const
{
	return mPhantom->getTransform();
}

void bsSceneNode::updateDerivedTransform() const
{
	//Get derivedTransform transform from parent
	if (mParentSceneNode)
	{
		//Get transform from parent, add own transform.
		//TODO: Make this not result in rediculously high translations
		//const hkTransform& localTransform = mPhantom->getTransform();//gjør om denne til mTransform?
		// = mParentSceneNode->getDerivedTransformation();

		//derivedTransform.getRotation().mul(localTransform.getRotation());
		//derivedTransform.getTranslation().add3clobberW(localTransform.getTranslation());
		//derivedTransform.getTranslation().setMul3(derivedTransform.getRotation(), derivedTransform.getTranslation());
			
		//Multiply own transform with derived.
		hkTransform derivedTransform;
		derivedTransform.setMul(mParentSceneNode->getDerivedTransformation(), mTransform);

		mPhantom->setTransform(derivedTransform);
	}
	else
	{
		//No parents to derive transform from, meaning own (local) transform is
		//already in world space.
		mPhantom->setTransform(mTransform);
	}
	/*
	//Add own (local) transform
	hkTransform oldTransform = mPhantom->getTransform();
	oldTransform.getTranslation().add3clobberW(mTransform.getTranslation());
	oldTransform.getRotation().mul(mTransform.getRotation());
	mPhantom->setTransform(oldTransform);
	*/
}

void bsSceneNode::setTranslation(const float x, const float y, const float z)
{
	hkVector4 translation(mTransform.getTranslation());
	translation.sub3clobberW(hkVector4(x, y, z, 0.0f));

	mTransform.getTranslation().set(x, y, z, 0.0f);

	//Get translation
	translation.sub3clobberW(mTransform.getTranslation());
	
	updateDerivedTransform();
}

void bsSceneNode::translate(const hkVector4& translation)
{
	mTransform.getTranslation().add3clobberW(translation);
	
	updateDerivedTransform();
}

void bsSceneNode::attachRenderable(const std::shared_ptr<bsRenderable>& renderable)
{
	assert(renderable->isOkForRendering());

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
			bsT::unordered_erase(mRenderables, mRenderables[i]);

			break;
		}
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
	hkVector4 halfExtents;
	mAabb.getHalfExtents(halfExtents);
	hkpBoxShape* shape = new hkpBoxShape(halfExtents);
	mPhantom->setShape(shape);
}
