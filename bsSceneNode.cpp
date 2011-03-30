#include "bsSceneNode.h"

#include "bsSceneGraph.h"
#include "bsResourceManager.h"
#include "bsOctNode.h"


bsSceneNode::bsSceneNode(const hkVector4& position, int id, bsSceneGraph* sceneGraph)
	: bsNode(position, id, sceneGraph)
	, mOctNode(nullptr)
	, mParentSceneNode(nullptr)
{
	mWireframeColor = XMFLOAT3(1.0f, 1.0f, 0.0f);

	mDerivedTransform.setIdentity();
}

bsSceneNode::~bsSceneNode()
{

}

bsSceneNode* bsSceneNode::createChild(const hkVector4& position /*= hkVector4(0.0f, 0.0f, 0.0f, 0.0f)*/)
{
	bsSceneNode* node = new bsSceneNode(position, mSceneGraph->getNumCreatedObjects(),
		mSceneGraph);
	mChildren.push_back(node);

	mSceneGraph->mSceneNodes.push_back(node);

	return node;
}

void bsSceneNode::verifyPosition()
{
	//Oct node 1 is the root node. It should always be able to contain this scene node.
	//It's probably current mOctNode because this scene node was intersecting multiple
	//smaller oct nodes.
	if (mOctNode->mID != 1 && mOctNode->mAABB.contains(mAABB))
	{
		//Still fits in current octNode's AABB, no change necessary.
		return;
	}

	mSceneGraph->placeSceneNode(this, mSceneGraph->mRootNode);
}

void bsSceneNode::draw(bsDx11Renderer* dx11Renderer) const
{
	for (unsigned int i = 0u; i < mRenderables.size(); ++i)
	{
		if (mRenderables[i]->getRenderableIdentifier() == bsRenderable::MESH)
		{
//			static_cast<bsMesh*>(mRenderables[i].get())->draw(dx11Renderer);
			//mRenderables[i]->draw(dx11Renderer);
		}
	}
}

void bsSceneNode::verifyAabb()
{
	bool changed = false;
	for (unsigned int i = 0u; i < mRenderables.size(); ++i)
	{
		if (mRenderables[i]->getRenderableIdentifier() == bsRenderable::MESH)
		{
			bsMesh* mesh = static_cast<bsMesh*>(mRenderables[i].get());
			if (!mAABB.contains(mesh->mAabb))
			{
				mAABB.includeAabb(mesh->mAabb);
				changed = true;
			}
		}
	}

	if (changed)
	{
		createDrawableAabb(mSceneGraph->mDx11Renderer,
			mSceneGraph->mResourceManager->getShaderManager(), false);
		mWireFramePrimitive->mColor = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
	}
}

const hkVector4& bsSceneNode::getDerivedTranslation() const
{
	if (mDerivedTransformNeedsUpdate)
	{
		updateDerivedTransform();
	}

	return mDerivedTransform.getTranslation();
}

const hkTransform& bsSceneNode::getDerivedTransformation() const
{
	if (mDerivedTransformNeedsUpdate)
	{
		updateDerivedTransform();
	}

	return mDerivedTransform;
}

void bsSceneNode::updateDerivedTransform() const
{
	if (mDerivedTransformNeedsUpdate)
	{
		//Get derived transform from parent
		if (mParentSceneNode)
		{
			mDerivedTransform = mParentSceneNode->getDerivedTransformation();
		}
		else
		{
			const hkTransform& octTransform = mOctNode->getTransformation();
			mDerivedTransform.getTranslation().add3clobberW(octTransform.getTranslation());
			mDerivedTransform.getRotation().mul(octTransform.getRotation());
		}

		//Add own (local) transform
		mDerivedTransform.getRotation().mul(mTransform.getRotation());
		mDerivedTransform.getTranslation().add3clobberW(mTransform.getTranslation());
	}

	mDerivedTransformNeedsUpdate = false;
}

void bsSceneNode::setTranslation(const float x, const float y, const float z)
{
	hkVector4 translation(mTransform.getTranslation());
	translation.sub3clobberW(hkVector4(x, y, z, 0.0f));

	mTransform.getTranslation().set(x, y, z, 0.0f);


	//Get translation
	translation.sub3clobberW(mTransform.getTranslation());

	//Translate the AABB
	mAABB.m_min.add3clobberW(translation);
	mAABB.m_max.add3clobberW(translation);

	mDerivedTransformNeedsUpdate = true;
	verifyTranslation();
}

void bsSceneNode::translate(const hkVector4& translation)
{
	mTransform.getTranslation().add3clobberW(translation);

	//Translate the AABB
	mAABB.m_min.add3clobberW(translation);
	mAABB.m_max.add3clobberW(translation);

	mDerivedTransformNeedsUpdate = true;
	verifyTranslation();
}
