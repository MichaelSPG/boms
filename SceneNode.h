#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include "Node.h"

#include <vector>
#include <memory>

#include "Renderable.h"
#include "Mesh.h"
#include "Templates.h"


class SceneGraph;
class OctNode;

class SceneNode : public Node
{
	friend class SceneGraph;
	friend class OctNode;
	friend class Application;//TODO: remove

	SceneNode(const hkVector4& position, int id, SceneGraph* sceneGraph);
	~SceneNode();

public:
	SceneNode* createChild(const hkVector4& position = hkVector4(0.0f, 0.0f, 0.0f, 0.0f));

	inline void attachRenderable(const std::shared_ptr<Mesh>& renderable)
	{
		mRenderables.push_back(renderable);
//		renderable->mSceneNode = this;
		verifyPosition();
		verifyAabb();
	}

	inline void detachRenderable(const std::shared_ptr<Renderable>& renderable)
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

	inline const std::vector<std::shared_ptr<Renderable>>& getRenderables() const
	{
		return mRenderables;
	}

	void draw(Dx11Renderer* dx11Renderer) const;

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

private:
	void verifyPosition();

	void verifyAabb();

	void updateDerivedTransform() const;

	std::vector<SceneNode*> mChildren;
	SceneNode*	mParentSceneNode;
	OctNode*	mOctNode;

	std::vector<std::shared_ptr<Renderable>>	mRenderables;

	//Mutable to allow const scene nodes to get derived transformations.
	mutable hkTransform	mDerivedTransform;
	mutable bool		mDerivedTransformNeedsUpdate;
};

#endif // SCENE_NODE_H