#include "SceneGraph.h"

#include <assert.h>
#include <vector>

#include "OctNode.h"
#include "SceneNode.h"
#include "Timer.h"
#include "ResourceManager.h"
#include "Camera.h"



SceneGraph::SceneGraph()
	: mNumCreatedObjects(0)
	, mDx11Renderer(nullptr)
	, mRootNode(nullptr)
	, mCamera(nullptr)
	, mResourceManager(nullptr)
	, displayEmptyAabbs(false)
{
}

SceneGraph::~SceneGraph()
{
	if (mRootNode)
	{
		//Recursive delete
		delete mRootNode;
	}
	for (unsigned int i = 0u; i < mSceneNodes.size(); ++i)
	{
		delete mSceneNodes[i];
	}
	if (mCamera)
	{
		delete mCamera;
	}
}

void SceneGraph::init(unsigned short treeDepth, Dx11Renderer* renderer,
	ResourceManager* resourceManager)
{
	assert(treeDepth != 0 && "Tree depth cannot be zero.");
	assert(renderer);
	assert(resourceManager);

	mDx11Renderer = renderer;
	mResourceManager = resourceManager;
	mMaxTreeDepth = treeDepth;

	hkAabb aabb(hkVector4(-100.0f, -100.0f, -100.0f), hkVector4(100.0f, 100.0f, 100.0f));

	mRootNode = new OctNode(getNumCreatedObjects(), hkVector4(0.0f, 0.0f, 0.0f), this,
		nullptr, aabb);

	mOctNodes.push_back(mRootNode);

	mRootNode->createChildren(0, this);
//	mRootNode->createDrawableAabb(mRenderer, shaderManager);

	/*
	SceneNode* node = createSceneNode();
	hkVector4 amin(-0.5f, -0.5f, -0.5f);
	hkVector4 amax( 0.5f,  0.5f,  0.5f);
	
	node->mAABB.m_min = amin;
	node->mAABB.m_max = amax;

	node->setPosition(10.0f, 10.0f, 30.0f);

	mSceneNodes.push_back(node);
	*/

	for (unsigned int i = 0u; i < mSceneNodes.size(); ++i)
	{
		mSceneNodes[i]->createDrawableAabb(mDx11Renderer, mResourceManager->getShaderManager());
	}

	for (unsigned int i = 0u; i < mOctNodes.size(); ++i)
	{
		mOctNodes[i]->createDrawableAabb(mDx11Renderer, mResourceManager->getShaderManager());
	}

	Log::logMessage("Scene graph initialized successfully");
}
#include <sstream>
void SceneGraph::drawAABBs(Dx11Renderer* dx11Renderer) const
{
	assert (dx11Renderer);

	for (unsigned int i = 0u; i < mSceneNodes.size(); ++i)
	{
		//TODO: Remove this
		/*
		std::stringstream msg;
		msg << "i: " << i << " | nodes: " << mSceneNodes.size();
		Log::logMessage(msg.str().c_str());
		*/

		CBWireFrame cb;
		XMStoreFloat4x4(&cb.world, XMMatrixIdentity());

		
		if (mSceneNodes[i]->mWireFramePrimitive)
		{
			//const hkVector4& translation = mSceneNodes[i]->getTranslation();
			const hkVector4& translation = mSceneNodes[i]->getDerivedTranslation();
			cb.world._14 = translation.getSimdAt(0);
			cb.world._24 = translation.getSimdAt(1);
			cb.world._34 = translation.getSimdAt(2);
			memcpy(&cb.color, &mSceneNodes[i]->mWireframeColor, 12);
			//cb.color.x = mSceneNodes[i]->mWireframeColor.x;
			//cb.color.y = mSceneNodes[i]->mWireframeColor.y;
			//cb.color.z = mSceneNodes[i]->mWireframeColor.z;
			cb.color.w = 0.0f;
			mDx11Renderer->getDeviceContext()->UpdateSubresource(mSceneNodes[i]
			->mWireFramePrimitive->mBuffer, 0, nullptr, &cb, 0, 0);
			
			
			mSceneNodes[i]->drawAABB(dx11Renderer);
		}
	}

	for (unsigned int i = 0u; i < mOctNodes.size(); ++i)
	{
		if (mOctNodes[i]->mSceneNodes.size())
		{
			mOctNodes[i]->drawAABB(dx11Renderer);
			/*
			for (unsigned int j = 0u; j < mOctNodes[i]->mSceneNodes.size(); ++j)
			{
				mOctNodes[i]->mSceneNodes[j]->draw(dx11Renderer);
			}
			*/
		}
		else
		{
			if (displayEmptyAabbs)
			{
				mOctNodes[i]->drawAABB(dx11Renderer);
			}
		}
	}
}

SceneNode* SceneGraph::createSceneNode(const hkVector4& position /*= hkVector4(0.0f, 0.0f, 0.0f, 0.0f)*/)
{
	SceneNode* node = new SceneNode(position, getNumCreatedObjects(), this);
	
	placeSceneNode(node, mRootNode, 1);

	mSceneNodes.push_back(node);

	return node;
}

void SceneGraph::placeSceneNode(SceneNode* sceneNode, OctNode* octNode, unsigned short depth)
{
	if (depth < mMaxTreeDepth)
	{
		if (octNode->mAABB.contains(sceneNode->mAABB))
		{
			OctNode* candidate = nullptr;

			for (unsigned int i = 0u; i < octNode->mChildren.size(); ++i)
			{
				if (octNode->mChildren[i]->mAABB.contains(sceneNode->mAABB))
				{
					if (!candidate)
					{
						candidate = octNode->mChildren[i];
					}
					else
					{
						//More than 1 candidate octNode for this sceneNode, meaning
						//it is contained by more than 1 octNode. Putting it in the
						//biggest node which can fit it completely.
						if (sceneNode->mOctNode
							&& (std::find(sceneNode->mOctNode->mSceneNodes.cbegin(),
							sceneNode->mOctNode->mSceneNodes.cend(), sceneNode)
							!= sceneNode->mOctNode->mSceneNodes.cend()))
						{
							sceneNode->mOctNode->detatchSceneNode(sceneNode);
						}
						
						octNode->attachSceneNode(sceneNode);
						return;
					}
				}
			}
			if (candidate)
			{
				placeSceneNode(sceneNode, candidate, ++depth);
			}
			else
			{
				//No candidates found, sceneNode is probably outside octree.
				//Put it in current octNode which should be the root.
				if (sceneNode->mOctNode
					&& (std::find(sceneNode->mOctNode->mSceneNodes.cbegin(),
					sceneNode->mOctNode->mSceneNodes.cend(), sceneNode)
					!= sceneNode->mOctNode->mSceneNodes.cend()))
				{
					sceneNode->mOctNode->detatchSceneNode(sceneNode);
				}
				octNode->attachSceneNode(sceneNode);
				return;
			}
		}
		else
		{
			if (sceneNode->mOctNode
				&& (std::find(sceneNode->mOctNode->mSceneNodes.cbegin(),
				sceneNode->mOctNode->mSceneNodes.cend(), sceneNode)
				!= sceneNode->mOctNode->mSceneNodes.cend()))
			{
				sceneNode->mOctNode->detatchSceneNode(sceneNode);
			}
			octNode->attachSceneNode(sceneNode);
			return;
		}

	}
	else
	{
		if (sceneNode->mOctNode
			&& (std::find(sceneNode->mOctNode->mSceneNodes.cbegin(),
			sceneNode->mOctNode->mSceneNodes.cend(), sceneNode)
			!= sceneNode->mOctNode->mSceneNodes.cend()))
		{
			sceneNode->mOctNode->detatchSceneNode(sceneNode);
		}
		octNode->attachSceneNode(sceneNode);
	}
}

const std::vector<std::shared_ptr<Renderable>> SceneGraph::getVisibleRenderables() const
{
	std::vector<std::shared_ptr<Renderable>> renderables;

	for (unsigned int i = 0u; i < mOctNodes.size(); ++i)
	{
		OctNode* currentOctNode = mOctNodes[i];
		unsigned int sceneNodeCount = currentOctNode->mSceneNodes.size();

		for (unsigned int j = 0u; j < sceneNodeCount; ++j)
		{
			const auto& sceneNodeRenderables = currentOctNode->mSceneNodes[j]->getRenderables();
			for (unsigned int k = 0u; k < sceneNodeRenderables.size(); ++k)
			{
				renderables.insert(renderables.end(), sceneNodeRenderables.begin(),
					sceneNodeRenderables.end());
			}
		}
	}

	return renderables;
}
