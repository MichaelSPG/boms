#include "bsSceneGraph.h"

#include <assert.h>
#include <vector>

#include "bsOctNode.h"
#include "bsSceneNode.h"
#include "bsTimer.h"
#include "bsResourceManager.h"
#include "bsCamera.h"
#include "bsLog.h"


bsSceneGraph::bsSceneGraph(unsigned short treeDepth, bsDx11Renderer* renderer,
	bsResourceManager* resourceManager)
	: displayEmptyAabbs(false)
	, mNumCreatedObjects(0)
	, mRootNode(nullptr)
	, mDx11Renderer(renderer)
	, mResourceManager(resourceManager)
	, mCamera(nullptr)
	, mMaxTreeDepth(treeDepth)
{
	assert(treeDepth != 0 && "Tree depth cannot be zero.");
	assert(renderer);
	assert(resourceManager);

	hkAabb aabb(hkVector4(-100.0f, -100.0f, -100.0f), hkVector4(100.0f, 100.0f, 100.0f));

	mRootNode = new bsOctNode(getNumCreatedObjects(), hkVector4(0.0f, 0.0f, 0.0f), this,
		nullptr, aabb);

	mOctNodes.push_back(mRootNode);

	mRootNode->createChildren(0, this);

	for (unsigned int i = 0u; i < mSceneNodes.size(); ++i)
	{
		mSceneNodes[i]->createDrawableAabb(mDx11Renderer, mResourceManager->getShaderManager());
	}

	for (unsigned int i = 0u; i < mOctNodes.size(); ++i)
	{
		mOctNodes[i]->createDrawableAabb(mDx11Renderer, mResourceManager->getShaderManager());
	}

	bsLog::logMessage("Scene graph initialized successfully");
}

bsSceneGraph::~bsSceneGraph()
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

#include <sstream>
void bsSceneGraph::drawAABBs(bsDx11Renderer* dx11Renderer) const
{
	assert (dx11Renderer);

	for (unsigned int i = 0u; i < mSceneNodes.size(); ++i)
	{
		//TODO: Remove this
		/*
		std::stringstream msg;
		msg << "i: " << i << " | nodes: " << mSceneNodes.size();
		bsLog::logMessage(msg.str().c_str());
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

bsSceneNode* bsSceneGraph::createSceneNode(const hkVector4& position /*= hkVector4(0.0f, 0.0f, 0.0f, 0.0f)*/)
{
	bsSceneNode* node = new bsSceneNode(position, getNumCreatedObjects(), this);
	
	placeSceneNode(node, mRootNode, 1);

	mSceneNodes.push_back(node);

	return node;
}

void bsSceneGraph::placeSceneNode(bsSceneNode* sceneNode, bsOctNode* octNode, unsigned short depth)
{
	if (depth < mMaxTreeDepth)
	{
		if (octNode->mAABB.contains(sceneNode->mAABB))
		{
			bsOctNode* candidate = nullptr;

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

const std::vector<std::shared_ptr<bsRenderable>> bsSceneGraph::getVisibleRenderables() const
{
	std::vector<std::shared_ptr<bsRenderable>> renderables;

	for (unsigned int i = 0u; i < mOctNodes.size(); ++i)
	{
		bsOctNode* currentOctNode = mOctNodes[i];
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
