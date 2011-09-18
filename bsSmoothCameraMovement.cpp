#include "StdAfx.h"

#include "bsSmoothCameraMovement.h"

#include "bsMath.h"
#include "bsSceneNode.h"
#include "bsCamera.h"


bsSmoothCameraMovement::bsSmoothCameraMovement(bsSceneNode* cameraNode,
	float maxForwardSpeed, float maxRightSpeed, float maxUpSpeed)
	: mCameraNode(cameraNode)

	, mCurrentForwardSpeed(0.0f)
	, mCurrentRightSpeed(0.0f)
	, mCurrentUpSpeed(0.0f)

	, mMaxForwardSpeed(maxForwardSpeed)
	, mMaxRightSpeed(maxRightSpeed)
	, mMaxUpSpeed(maxUpSpeed)
{
}

bsSmoothCameraMovement::~bsSmoothCameraMovement()
{
}

void bsSmoothCameraMovement::update(float forwardSpeed, float rightSpeed, float upSpeed,
	float deltaTimeMs)
{
	//TODO: Change this to something more sensible.
	const float ratio = deltaTimeMs / 75.0f;
	
	//Lerp current speed to accelerate movement.
	mCurrentForwardSpeed =	bsMath::lerp(mCurrentForwardSpeed, forwardSpeed, ratio);
	mCurrentRightSpeed =	bsMath::lerp(mCurrentRightSpeed, rightSpeed, ratio);
	mCurrentUpSpeed =		bsMath::lerp(mCurrentUpSpeed, upSpeed, ratio);

	//Make sure we're not moving faster than our max speed.
	mCurrentForwardSpeed =	bsMath::clamp(-mMaxForwardSpeed, mMaxForwardSpeed, mCurrentForwardSpeed);
	mCurrentRightSpeed =	bsMath::clamp(-mMaxRightSpeed, mMaxRightSpeed, mCurrentRightSpeed);
	mCurrentUpSpeed =		bsMath::clamp(-mMaxUpSpeed, mMaxUpSpeed, mCurrentUpSpeed);


	XMVECTOR translation = XMVectorSet(mCurrentRightSpeed, mCurrentUpSpeed,
		mCurrentForwardSpeed, 0.0f);

	//Rotate the translation to be aligned with the camera's rotation.
	const XMVECTOR& cameraRotation = mCameraNode->getRotation();
	translation = XMVector3Rotate(translation, XMQuaternionInverse(cameraRotation));

	mCameraNode->translate(translation);
}
