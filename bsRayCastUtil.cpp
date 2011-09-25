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

void bsRayCastUtil::castRay(const XMVECTOR& origin, const XMVECTOR& forward, float rayLength,
	hkpWorld* world, hkpWorldRayCastOutput& hitCollectorOut, XMVECTOR& destinationOut)
{
	BS_ASSERT2(bsMath::approximatelyEqual(XMVectorGetX(XMVector3LengthSq(forward)), 1.0f),
		"Forward vector is not normalized.");

	//Calculate destination point of ray.
	const XMVECTOR destination = XMVectorAdd(origin, XMVectorScale(forward, rayLength));
	destinationOut = destination;

	return castRay(origin, destination, world, hitCollectorOut);
}

XMVECTOR bsRayCastUtil::screenSpaceToObjectSpace(const XMVECTOR& screenPoint,
	const XMFLOAT2& screenSize, const XMMATRIX& projection, const XMMATRIX& view)
{
	return XMVector3Unproject(screenPoint,
		0.0f, 0.0f,//Only for rendering subsets of viewport.
		screenSize.x, screenSize.y,
		0.0f, 1.0f,//Range of depth when rendering.
		projection, view, XMMatrixIdentity());
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
