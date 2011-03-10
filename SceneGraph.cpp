#include "SceneGraph.h"

#include <assert.h>
#include <vector>

#include "OctNode.h"
#include "SceneNode.h"
#include "Timer.h"
#include "ShaderManager.h"
#include "Camera.h"



SceneGraph::SceneGraph()
	: mNumCreatedObjects(0)
	, mRenderer(nullptr)
	, mRootNode(nullptr)
	, mCamera(nullptr)
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
	ShaderManager* shaderManager)
{
	assert(treeDepth != 0 && "Tree depth cannot be zero.");
	assert(renderer);

	mRenderer = renderer;
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
		mSceneNodes[i]->createDrawableAabb(mRenderer, shaderManager);
	}

	for (unsigned int i = 0u; i < mOctNodes.size(); ++i)
	{
		mOctNodes[i]->createDrawableAabb(mRenderer, shaderManager);
	}

	Log::logMessage("Scene graph initialized successfully");
}

void SceneGraph::drawAABBs(Dx11Renderer* dx11Renderer) const
{
	assert (dx11Renderer);


	for (unsigned int i = 0u; i < mSceneNodes.size(); ++i)
	{
		mSceneNodes[i]->drawAABB(dx11Renderer);
	}

	for (unsigned int i = 0u; i < mOctNodes.size(); ++i)
	{
		if (mOctNodes[i]->mSceneNodes.size())
		{
			mOctNodes[i]->drawAABB(dx11Renderer);
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
