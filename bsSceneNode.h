#ifndef BS_SCENE_NODE_H
#define BS_SCENE_NODE_H

#include "bsNode.h"

#include <vector>
#include <memory>

#include "bsRenderable.h"
#include "bsMesh.h"
#include "bsTemplates.h"


class bsSceneGraph;
class bsOctNode;

class bsSceneNode : public bsNode
{
	friend class bsSceneGraph;
	friend class bsOctNode;
	friend class Application;//TODO: remove

	bsSceneNode(const hkVector4& position, int id, bsSceneGraph* sceneGraph);
	~bsSceneNode();

public:
	bsSceneNode* createChild(const hkVector4& position = hkVector4(0.0f, 0.0f, 0.0f, 0.0f));

	inline void attachRenderable(const std::shared_ptr<bsMesh>& renderable)
	{
		mRenderables.push_back(renderable);
//		renderable->mSceneNode = this;
		verifyPosition();
		verifyAabb();
	}

	inline void detachRenderable(const std::shared_ptr<bsRenderable>& renderable)
	{
//		renderable->mSceneNode = nullptr;
		for (unsigned int i = 0u; i < mRenderables.size(); ++i)
		{
			if (mRenderables[i] == renderable)
			{
				unordered_erase(mRenderables, mRenderables[i]);
			}
		}
	}

	inline const std::vector<std::shared_ptr<bsRenderable>>& getRenderables() const
	{
		return mRenderables;
	}

	void draw(bsDx11Renderer* dx11Renderer) const;

	//World space
	const hkVector4& getDerivedTranslation() const;

	//Local space
	inline const hkTransform& getTransformation() const
	{
		return mTransform;
	}

	//World space
	const hkTransform& getDerivedTransformation() const;

	void setTranslation(const float x, const float y, const float z);

	//Calls the hkVector4 version of the same function
	inline void translate(const float x, const float y, const float z)
	{
		translate(hkVector4(x, y, z, 0.0f));
	}

	void translate(const hkVector4& translation);

	void setRotation(const hkRotation& rotation)
	{
		mTransform.setRotation(rotation);
	}

	void setRotation(const hkQuaternion& rotation)
	{
		mTransform.setRotation(rotation);
	}

private:
	void verifyPosition();

	void verifyAabb();

	void updateDerivedTransform() const;

	std::vector<bsSceneNode*> mChildren;
	bsSceneNode*	mParentSceneNode;
	bsOctNode*	mOctNode;

	std::vector<std::shared_ptr<bsRenderable>>	mRenderables;

	//Mutable to allow const scene nodes to get derived transformations.
	mutable hkTransform	mDerivedTransform;
	mutable bool		mDerivedTransformNeedsUpdate;
};

#endif // BS_SCENE_NODE_H
