#include "StdAfx.h"

#include "bsCharacterController.h"

#define setZero4 setZero
#define isOk3 isOk<3>
#define isOk4 isOk<4>
#define isNormalized3 isNormalized<3>
#include <Physics/Utilities/CharacterControl/hkpCharacterControl.h>
#undef setZero4
#undef isOk3
#undef isOk4
#undef isNormalized3

#include <Physics/Utilities/CharacterControl/FirstPersonCharacter/hkpFirstPersonCharacter.h>
#define setZero4 setZero
#include <Physics/Utilities/CharacterControl/CharacterRigidBody/hkpCharacterRigidBody.h>
#undef setZero4
#include <Physics/Utilities/CharacterControl/CharacterRigidBody/hkpCharacterRigidBodyListener.h>

#include <Physics/Utilities/CharacterControl/StateMachine/hkpCharacterContext.h>
#include <Physics/Utilities/CharacterControl/StateMachine/hkpCharacterStateManager.h>
#include <Physics/Utilities/CharacterControl/StateMachine/hkpDefaultCharacterStates.h>
#include <Physics/Collide/Shape/Convex/Capsule/hkpCapsuleShape.h>



bsCharacterController::bsCharacterController(const bsCharacterControllerCInfo& cInfo)
{
	hkpCharacterRigidBodyCinfo charCinfo;
	charCinfo.m_shape = new hkpCapsuleShape(hkVector4::getZero(),
		hkVector4(0.0f, cInfo.capsuleHeight, 0.0f), cInfo.capsuleRadius);
	charCinfo.m_mass = cInfo.mass;
	charCinfo.m_maxLinearVelocity = cInfo.maxRunSpeed;
	//charCinfo.m_maxForce = cInfo.maxPushForce;
	//charCinfo.m_maxLinearVelocity = cInfo.maxRunSpeed;
	//charCinfo.m_friction = 0.5f;

	mCharacterRigidBody = new hkpCharacterRigidBody(charCinfo);


	// Create the Character state machine and context.
	hkpCharacterState* state;
	mStateManager = new hkpCharacterStateManager();

	state = new hkpCharacterStateOnGround();
	static_cast<hkpCharacterStateOnGround*>(state)->setSpeed(cInfo.maxRunSpeed);
	mStateManager->registerState(state, HK_CHARACTER_ON_GROUND);
	state->removeReference();

	state = new hkpCharacterStateInAir();
	static_cast<hkpCharacterStateInAir*>(state)->setSpeed(cInfo.airSpeed);
	mStateManager->registerState(state, HK_CHARACTER_IN_AIR);
	state->removeReference();

	state = new hkpCharacterStateJumping();
	static_cast<hkpCharacterStateJumping*>(state)->setJumpHeight(cInfo.jumpHeight);
	mStateManager->registerState(state, HK_CHARACTER_JUMPING);
	state->removeReference();

	mCharacterContext = new hkpCharacterContext(mStateManager, HK_CHARACTER_IN_AIR);
	mStateManager->removeReference();

	// Set character type
	mCharacterContext->setCharacterType(hkpCharacterContext::HK_CHARACTER_RIGIDBODY);

	
	hkpFirstPersonCharacterCinfo fpsCinfo;
	fpsCinfo.m_capsuleHeight = cInfo.capsuleHeight;
	fpsCinfo.m_capsuleRadius = cInfo.capsuleRadius;
	fpsCinfo.m_characterRb = mCharacterRigidBody;
	fpsCinfo.m_context = mCharacterContext;
	fpsCinfo.m_eyeHeight = cInfo.eyeHeight;
	fpsCinfo.m_world = cInfo.world;

	fpsCinfo.m_position.setZero();
	fpsCinfo.m_direction.set(0.0f, 0.0f, 1.0f);
	fpsCinfo.m_gravityStrength = cInfo.gravityStrength;
	fpsCinfo.m_maxUpDownAngle = XM_PI * 0.4f;
	fpsCinfo.m_up.set(0.0f, 1.0f, 0.0f);
	
	mFirstPersonCharacter = new hkpFirstPersonCharacter(fpsCinfo);
	mCharacterRigidBody->m_unweldingHeightOffsetFactor = 2.0f;
	mCharacterRigidBody->m_maxSlopeCosine = cosf(XMConvertToRadians(80.0f));

	cInfo.world->markForWrite();
	cInfo.world->addEntity(mCharacterRigidBody->getRigidBody());

	hkpCharacterRigidBodyListener* listener = new hkpCharacterRigidBodyListener();
	mCharacterRigidBody->setListener(listener);
	listener->removeReference();

	cInfo.world->unmarkForWrite();
}

bsCharacterController::~bsCharacterController()
{
	mCharacterRigidBody->removeReference();
}

void bsCharacterController::step(float deltaTimeMs, float forwardBack, float leftRight,
	bool wantJump, const hkVector4& forwardDirection)
{
	hkpWorld* world = mCharacterRigidBody->getRigidBody()->getWorld();
	world->markForWrite();

	hkpFirstPersonCharacter::CharacterControls controls;
	controls.m_fire = false;
	controls.m_fireRmb = false;
	controls.m_forward = forwardDirection;
	controls.m_forwardBack = -forwardBack;
	controls.m_straffeLeftRight = -leftRight;
	controls.m_wantJump = wantJump;

	mFirstPersonCharacter->update(deltaTimeMs * 0.001f, controls);

	world->unmarkForWrite();
}

hkpRigidBody* bsCharacterController::getRigidBody() const
{
	return mCharacterRigidBody->getRigidBody();
}
