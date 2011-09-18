#include "StdAfx.h"

#include "bsScene.h"

#include <vector>

#include <Physics/Dynamics/World/hkpSimulationIsland.h>

#include "bsSceneNode.h"
#include "bsTimer.h"
#include "bsResourceManager.h"
#include "bsCamera.h"
#include "bsLog.h"
#include "bsAssert.h"
#include "bsHavokManager.h"
#include "bsCoreCInfo.h"


bsScene::bsScene(bsDx11Renderer* renderer, bsResourceManager* resourceManager,
	bsHavokManager* havokManager, const bsCoreCInfo& cInfo)
	: mNumCreatedNodes(0)
	, mDx11Renderer(renderer)
	, mResourceManager(resourceManager)
	, mPhysicsWorld(nullptr)
	, mHavokManager(havokManager)
{
	BS_ASSERT(renderer);
	BS_ASSERT(resourceManager);
	BS_ASSERT(havokManager);

	havokManager->createWorld(true);

	mPhysicsWorld = havokManager->getPhysicsWorld();

	bsProjectionInfo projectionInfo(45.0f, 1000.0f, 0.1f,
		(float)cInfo.windowWidth / (float)cInfo.windowHeight, (float)cInfo.windowWidth,
		(float)cInfo.windowHeight);
	mCamera = new bsCamera(projectionInfo, mDx11Renderer);
	mCamera->setScene(this);

	bsSceneNode* cameraNode = new bsSceneNode();
	cameraNode->getEntity().attach(mCamera);
	addSceneNode(cameraNode);

	bsLog::logMessage("Scene graph initialized successfully");
}

bsScene::~bsScene()
{
	for (unsigned int i = 0; i < mSceneNodes.size(); ++i)
	{
		delete mSceneNodes[i];
	}
	if (mCamera)
	{
		delete mCamera;
	}
}

void bsScene::addSceneNode(bsSceneNode* sceneNode)
{
	BS_ASSERT(sceneNode);

	sceneNode->addedToScene(this, getNewId());

	mSceneNodes.push_back(sceneNode);

	bsEntity& entity = sceneNode->getEntity();
	hkpRigidBody* rigidBody = entity.getComponent<hkpRigidBody*>();
	if (rigidBody != nullptr)
	{
		mPhysicsWorld->markForWrite();
		mPhysicsWorld->addEntity(rigidBody);
		mPhysicsWorld->unmarkForWrite();
	}
}

void bsScene::update(float deltaTime)
{
	mHavokManager->stepWorld(deltaTime);

	mCamera->update();

	synchronizeActiveEntities();
}

void bsScene::synchronizeActiveEntities()
{
	mPhysicsWorld->markForRead();
	const hkArray<hkpSimulationIsland*>& activeIslands = mPhysicsWorld->getActiveSimulationIslands();

	std::vector<bsSceneNode*> activeSceneNodes;
	//Guess number of active entities, save some reallocation.
	activeSceneNodes.reserve(activeIslands.getSize() * 10);

	for (int i = 0; i < activeIslands.getSize(); ++i)
	{
		const hkArray<hkpEntity*>& entities = activeIslands[i]->getEntities();

		for (int j = 0; j < entities.getSize(); ++j)
		{
			const hkpEntity* entity = entities[j];

			BS_ASSERT(entity->hasProperty(BSPK_ENTITY_POINTER));

			bsEntity* node = static_cast<bsEntity*>
				(entity->getProperty(BSPK_ENTITY_POINTER).getPtr());

			activeSceneNodes.push_back(node->getOwner());
		}
	}

	for (size_t i = 0; i < activeSceneNodes.size(); ++i)
	{
		activeSceneNodes[i]->setTransformFromRigidBody();
	}

	mPhysicsWorld->unmarkForRead();
}
