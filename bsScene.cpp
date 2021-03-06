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
#include "bsDx11Renderer.h"


bsScene::bsScene(bsDx11Renderer* renderer, bsHavokManager* havokManager,
	const bsCoreCInfo& cInfo)
	: mNumCreatedEntities(0)
	, mDx11Renderer(renderer)
	, mPhysicsWorld(nullptr)
	, mHavokManager(havokManager)
	, mStepPhysics(true)
	, mTimeScale(1.0f)
	, mPhysicsFrequency(60.0f)
	, mPhysicsStepSizeMs(1000.0f / 60.0f)
{
	BS_ASSERT(renderer);
	BS_ASSERT(havokManager);

	//Create physics world and visual debugger.
	createPhysicsWorld(mHavokManager->getJobQueue());
	mHavokManager->createVisualDebuggerForWorld(*mPhysicsWorld);

	//Create the camera.
	bsProjectionInfo projectionInfo(60.0f, 1000.0f, 0.1f,
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
	mHavokManager->destroyVisualDebuggerForWorld(*mPhysicsWorld);

	//Delete all entities in this scene.
	while (!mEntities.empty())
	{
		bsEntity& entity = *mEntities.back();

		removeEntityAndChildrenRecursively(entity, true);
	}

	mPhysicsWorld->markForWrite();
	mPhysicsWorld->removeContactListener(&mContactCounter);
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
	BS_ASSERT2(entityToRemove.getScene() == this, "Trying to remove an entity from a scene"
		" it is not a part of");

	auto itr = std::find(std::begin(mEntities), std::end(mEntities), &entityToRemove);

	//Verify that the entity was found in this scene.
	if (itr == mEntities.end())
	{
		BS_ASSERT2(false, "Trying to remove an entity from a scene it does not exist in");
		return;
	}

	bsTransform* parent = entityToRemove.getTransform().getParentTransform();
	if (parent != nullptr)
	{
		//Remove from parent, preventing the parent from keep an invalid reference to the
		//entity after it's been deleted.
		parent->unparentChild(entityToRemove.getTransform());
	}

	bs::unordered_erase(mEntities, itr);

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

	const float preStep = timer.getTimeMilliSeconds();

	unsigned int numPhysicsSteps = 0;

	if (mStepPhysics)
	{
		mPhysicsWorld->setFrameTimeMarker(deltaTimeMs * 0.001f * mTimeScale);

		const hkpStepResult result = mPhysicsWorld->advanceTime();
		BS_ASSERT(result == HK_STEP_RESULT_SUCCESS);

		while (!mPhysicsWorld->isSimulationAtMarker())
		{
			BS_ASSERT(mPhysicsWorld->isSimulationAtPsi());

			//pre-step callbacks

			mHavokManager->stepWorld(*mPhysicsWorld, mPhysicsStepSizeMs);
			mHavokManager->stepVisualDebugger(mPhysicsStepSizeMs);

			++numPhysicsSteps;

			if (mPhysicsWorld->isSimulationAtPsi())
			{
				//sync entities with rbs (maybe)
			}
		}
	}

	const float preSynchronize = timer.getTimeMilliSeconds();
	unsigned int totalActiveRigidBodies, totalActiveSimulationIslands;
	synchronizeActiveEntities(&totalActiveRigidBodies, &totalActiveSimulationIslands);

	framStatistics.physicsInfo.synchronizationDuration = timer.getTimeMilliSeconds() - preSynchronize;
	framStatistics.physicsInfo.stepDuration = preSynchronize - preStep;

	framStatistics.physicsInfo.numSteps = numPhysicsSteps;
	framStatistics.physicsInfo.numActiveRigidBodies = totalActiveRigidBodies;
	framStatistics.physicsInfo.numActiveSimulationIslands = totalActiveSimulationIslands;
	framStatistics.physicsInfo.numContacts = mContactCounter.getNumContacts();

	mCamera->update();
}

void bsScene::createPhysicsWorld(hkJobQueue& jobQueue)
{
	hkpWorldCinfo worldCinfo;
	worldCinfo.setBroadPhaseWorldSize(1000.0f);

	{
		worldCinfo.setupSolverInfo(hkpWorldCinfo::SOLVER_TYPE_8ITERS_HARD);
		worldCinfo.m_solverMicrosteps = 3;
		//worldCinfo.m_contactPointGeneration = hkpWorldCinfo::CONTACT_POINT_ACCEPT_ALWAYS;
		//worldCinfo.m_contactPointGeneration = hkpWorldCinfo::CONTACT_POINT_REJECT_DUBIOUS;
	}

	worldCinfo.m_solverIterations;
	worldCinfo.m_enableDeactivation;
	worldCinfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_MULTITHREADED;
	worldCinfo.m_broadPhaseBorderBehaviour = hkpWorldCinfo::BROADPHASE_BORDER_REMOVE_ENTITY;

	mPhysicsWorld = new hkpWorld(worldCinfo);
	mPhysicsWorld->markForWrite();

	hkpAgentRegisterUtil::registerAllAgents(mPhysicsWorld->getCollisionDispatcher());
	mPhysicsWorld->registerWithJobQueue(&jobQueue);

	mPhysicsWorld->addContactListener(&mContactCounter);

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
			//Can use unchecked here as non-rigid body entites are not in simulation islands.
			const hkpRigidBody* rigidBody = hkpGetRigidBodyUnchecked(entities[j]->getCollidable());
			
			++numActiveRigidBodies;

			bsEntity& entity = bsGetEntity(*rigidBody);
			//entity.getTransform().setTransformFromRigidBody(bsMath::toXM(rigidBody->getPosition()),
			//	bsMath::toXM(rigidBody->getRotation()));

			{
				hkTransform transform;
				rigidBody->approxCurrentTransform(transform);

				const XMVECTOR position = bsMath::toXM(transform.getTranslation());
				const XMVECTOR rotation = bsMath::toXM(transform.getRotation());

				entity.getTransform().setTransformFromRigidBody(position, rotation);
			}
		}
	}

	mPhysicsWorld->unmarkForRead();

	*totalActiveRigidBodies = numActiveRigidBodies;
}

void bsScene::removeEntityAndChildrenRecursively(bsEntity& entityToRemove, bool deleteAfterRemoving)
{
	const std::vector<bsTransform*>& children = entityToRemove.getTransform().getChildren();
	for (unsigned int i = 0, count = children.size(); i < count; ++i)
	{
		removeEntityAndChildrenRecursively(children[i]->getEntity(), deleteAfterRemoving);
	}

	removeEntity(entityToRemove);
	
	if (deleteAfterRemoving)
	{
		delete &entityToRemove;
	}
}
