#include "StdAfx.h"

#include "bsScene.h"

#include <vector>

#include <Physics/Dynamics/hkpDynamics.h>
#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Dynamics/World/hkpSimulationIsland.h>
#include <Physics/Collide/Dispatch/hkpAgentRegisterUtil.h>

#include "bsEntity.h"
#include "bsTimer.h"
#include "bsCamera.h"
#include "bsLog.h"
#include "bsAssert.h"
#include "bsHavokManager.h"
#include "bsCoreCInfo.h"
#include "bsMath.h"
#include "bsTemplates.h"
#include "bsFrameStatistics.h"


bsScene::bsScene(bsDx11Renderer* renderer, bsHavokManager* havokManager,
	const bsCoreCInfo& cInfo)
	: mNumCreatedEntities(0)
	, mDx11Renderer(renderer)
	, mPhysicsWorld(nullptr)
	, mHavokManager(havokManager)
	, mStepPhysics(true)
{
	BS_ASSERT(renderer);
	BS_ASSERT(havokManager);

	//Create physics world and visual debugger.
	createPhysicsWorld(mHavokManager->getJobQueue());
	mHavokManager->createVisualDebuggerForWorld(*mPhysicsWorld);

	//Create the camera.
	bsProjectionInfo projectionInfo(45.0f, 1000.0f, 0.1f,
		(float)cInfo.windowWidth / (float)cInfo.windowHeight, (float)cInfo.windowWidth,
		(float)cInfo.windowHeight);
	mCamera = new bsCamera(projectionInfo, mDx11Renderer);
	mCamera->setScene(this);

	bsEntity* cameraEntity = new bsEntity();
	cameraEntity->attachCamera(*mCamera);;
	addEntity(*cameraEntity);

	//Add resize listener to adjust camera's projection info when the screen is resized.
	mDx11Renderer->addResizeListener(
		[this](unsigned int screenWidth, unsigned int screenHeight)
	{
		bsProjectionInfo projectionInfo = mCamera->getProjectionInfo();
		projectionInfo.mScreenSize.x = (float)screenWidth;
		projectionInfo.mScreenSize.y = (float)screenHeight;
		mCamera->setProjectionInfo(projectionInfo);
	});

	bsLog::log("Scene graph initialized successfully");
}

bsScene::~bsScene()
{
	//Delete all entities in this scene. Cannot do this in a for loop
	while (!mEntities.empty())
	{
		bsEntity* entity = mEntities.back();
		entity->removedFromScene(*this);

		delete entity;
		mEntities.pop_back();
	}

	mHavokManager->destroyVisualDebuggerForWorld(*mPhysicsWorld);

	mPhysicsWorld->markForWrite();
	mPhysicsWorld->removeReference();

	//Camera is attached to an entity and will be deleted by that entity.
}

void bsScene::addEntity(bsEntity& entity)
{
	mEntities.push_back(&entity);

	entity.addedToScene(*this, getNewId());

	//Add the entity's rigid body (if one is present) to the physics simulation.
	hkpRigidBody* rigidBody = entity.getRigidBody();
	if (rigidBody != nullptr)
	{
		mPhysicsWorld->markForWrite();
		mPhysicsWorld->addEntity(rigidBody);
		mPhysicsWorld->unmarkForWrite();
	}
}

void bsScene::removeEntity(bsEntity& entityToRemove)
{
	BS_ASSERT2(entityToRemove.getScene() == this, "Trying to remove en entity from a scene"
		" it is not a part of");

	const auto itr = std::find(std::begin(mEntities), std::end(mEntities), &entityToRemove);

	//Verify that the entity was found in this scene.
	BS_ASSERT2(itr != std::end(mEntities),
		"Trying to remove an entity from a scene it does not exist in");

	bs::unordered_erase(mEntities, *itr);

	entityToRemove.removedFromScene(*this);

	//Remove the entity's rigid body (if one is present) to the physics simulation.
	hkpRigidBody* rigidBody = entityToRemove.getRigidBody();

	//Need to check that the world is not null in case the rigid body fell out of the
	//broadphase and was removed from it by Havok.
	if (rigidBody != nullptr && rigidBody->getWorld() != nullptr)
	{
		mPhysicsWorld->markForWrite();

		//Activate the rigid body and its simulation island, so that if objects were
		//resting on the rigid body, they will become active and fall down rather than
		//be frozen in the air.
		rigidBody->activate();

		mPhysicsWorld->removeEntity(rigidBody);
		mPhysicsWorld->unmarkForWrite();
	}
}

void bsScene::update(float deltaTimeMs, bsFrameStatistics& framStatistics)
{
	bsTimer timer;

	float preStep = timer.getTimeMilliSeconds();
	if (mStepPhysics)
	{
		mHavokManager->stepWorld(*mPhysicsWorld, deltaTimeMs);
	}

	framStatistics.physicsInfo.stepDuration = timer.getTimeMilliSeconds() - preStep;

	mCamera->update();

	float preSynchronize = timer.getTimeMilliSeconds();
	unsigned int totalActiveRigidBodies, totalActiveSimulationIslands;
	synchronizeActiveEntities(&totalActiveRigidBodies, &totalActiveSimulationIslands);

	framStatistics.physicsInfo.synchronizationDuration = timer.getTimeMilliSeconds() - preSynchronize;
	framStatistics.physicsInfo.numActiveRigidBodies = totalActiveRigidBodies;
	framStatistics.physicsInfo.numActiveSimulationIslands = totalActiveSimulationIslands;
}

void bsScene::createPhysicsWorld(hkJobQueue& jobQueue)
{
	hkpWorldCinfo worldCinfo;
	worldCinfo.setBroadPhaseWorldSize(1000.0f);
	worldCinfo.m_solverIterations;
	worldCinfo.m_enableDeactivation;
	worldCinfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_MULTITHREADED;
	worldCinfo.m_broadPhaseBorderBehaviour = hkpWorldCinfo::BROADPHASE_BORDER_REMOVE_ENTITY;

	mPhysicsWorld = new hkpWorld(worldCinfo);
	mPhysicsWorld->markForWrite();

	hkpAgentRegisterUtil::registerAllAgents(mPhysicsWorld->getCollisionDispatcher());
	mPhysicsWorld->registerWithJobQueue(&jobQueue);

	mPhysicsWorld->unmarkForWrite();
}

void bsScene::synchronizeActiveEntities(unsigned int* totalActiveRigidBodies,
	unsigned int* totalActiveSimulationIslands)
{
	BS_ASSERT(totalActiveRigidBodies);
	BS_ASSERT(totalActiveSimulationIslands);

	/*	This function synchronizes every rigid body which is in an active simulation
		island with its scene entity.  Inactive simulation islands are not simulated, and
		are therefore not required to be synchronized.
	*/

	unsigned int numActiveRigidBodies = 0;

	mPhysicsWorld->markForRead();
	const hkArray<hkpSimulationIsland*>& activeIslands = mPhysicsWorld->getActiveSimulationIslands();
	*totalActiveSimulationIslands = activeIslands.getSize();

	for (int i = 0; i < activeIslands.getSize(); ++i)
	{
		const hkArray<hkpEntity*>& entities = activeIslands[i]->getEntities();

		for (int j = 0; j < entities.getSize(); ++j)
		{
			hkpRigidBody* rigidBody = hkpGetRigidBody(entities[j]->getCollidable());
			BS_ASSERT(rigidBody->hasProperty(BSPK_ENTITY_POINTER));
			
			if (rigidBody != nullptr)
			{
				++numActiveRigidBodies;

				bsEntity* entity = static_cast<bsEntity*>
					(rigidBody->getProperty(BSPK_ENTITY_POINTER).getPtr());

				entity->getTransform().setTransformFromRigidBody(bsMath::toXM(rigidBody->getPosition()),
					bsMath::toXM(rigidBody->getRotation()));
			}
		}
	}

	mPhysicsWorld->unmarkForRead();

	*totalActiveRigidBodies = numActiveRigidBodies;
}
