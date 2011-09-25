#pragma once

class hkpWorld;
class hkpCharacterRigidBody;
class hkpRigidBody;
class hkVector4;
class hkpCharacterStateManager;
class hkpCharacterContext;
struct hkpSurfaceInfo;
class hkpFirstPersonCharacter;


struct bsCharacterControllerCInfo
{
	bsCharacterControllerCInfo()
		: maxRunSpeed(20.0f)
		, airSpeed(10.0f)
		, jumpHeight(10.0f)
		, mass(50.0f)
		, maxPushForce(50.0f)

		, capsuleHeight(1.2f)
		, capsuleRadius(0.3f)

		, eyeHeight(1.1f)

		, world(nullptr)
		, gravityStrength(1.0f)
	{}

	float maxRunSpeed;
	float airSpeed;
	float jumpHeight;
	float mass;
	float maxPushForce;

	float capsuleHeight;
	float capsuleRadius;

	float eyeHeight;

	float gravityStrength;

	hkpWorld* world;
};


class bsCharacterController
{
public:
	bsCharacterController(const bsCharacterControllerCInfo& cInfo);

	~bsCharacterController();

	
	void step(float deltaTimeMs, float forwardBack, float leftRight, bool wantJump,
		const hkVector4& forwardDirection);

	hkpRigidBody* getRigidBody() const;

private:
	hkpCharacterRigidBody*		mCharacterRigidBody;
	hkpCharacterStateManager*	mStateManager;
	hkpCharacterContext*		mCharacterContext;
	hkpFirstPersonCharacter*	mFirstPersonCharacter;
};
