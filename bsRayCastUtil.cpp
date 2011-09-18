#include "StdAfx.h"

#include "bsRayCastUtil.h"
#include "bsAssert.h"
#include "bsMath.h"

#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastInput.h>
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastOutput.h>


void bsRayCastUtil::castRay(const XMVECTOR& origin, const XMVECTOR& destination,
	hkpWorld* world, hkpWorldRayCastOutput& hitCollectorOut)
{
	BS_ASSERT(world != nullptr);

	hkpWorldRayCastInput input;
	input.m_from = bsMath::toHK(origin);
	input.m_to = bsMath::toHK(destination);
	
	world->lock();
	world->castRay(input, hitCollectorOut);
	world->unlock();
}

void bsRayCastUtil::castRay(const XMVECTOR& origin, const XMVECTOR& rotation,
	float rayLength, hkpWorld* world, hkpWorldRayCastOutput& hitCollectorOut, XMVECTOR& destinationOut)
{
	//Translation of ray from origin.
	XMVECTOR rayTranslation = XMVectorSet(0.0f, 0.0f, rayLength, 0.0f);
	//Rotate the translation to match the input rotation.
	rayTranslation = XMVector3InverseRotate(rayTranslation, rotation);

	//const XMVECTOR destination = XMVectorAdd(origin, rayTranslation);

	//return castRay(origin, destination, hitCollector, world);


	const XMVECTOR trans = XMVectorSet(0.0f, 0.0f, rayLength, 0.0f);

	const XMVECTOR rotatedTrans = XMVector3InverseRotate(trans, rotation);

	const XMVECTOR destination = XMVectorAdd(origin, rotatedTrans);

	destinationOut = destination;

	return castRay(origin, destination, world, hitCollectorOut);
}

XMVECTOR bsRayCastUtil::screenSpaceToObjectSpace(const XMVECTOR& screenPoint,
	const XMFLOAT2& screenSize, const XMMATRIX& projection, const XMMATRIX& inverseView)
{
	const XMVECTOR objectSpaceCursorPos = XMVector3Unproject(screenPoint,
		0.0f, 0.0f,//Only for rendering subsets of viewport.
		screenSize.x, screenSize.y,
		0.0f, 1.0f,//Range of depth when rendering.
		projection, inverseView, XMMatrixIdentity());

	return objectSpaceCursorPos;
}

hkVector4 bsRayCastUtil::getHitPosition(const hkpWorldRayCastInput& input,
	const hkpWorldRayCastOutput& output)
{
	BS_ASSERT2(output.hasHit(), "Cannot find hit position for outputs with no hit.");

	hkVector4 hitPosition;
	hitPosition.setInterpolate(input.m_from, input.m_to,
		hkSimdReal::convert(output.m_hitFraction));

	return hitPosition;
}
