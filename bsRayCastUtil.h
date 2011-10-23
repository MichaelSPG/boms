#pragma once

#include <Windows.h>
#include <xnamath.h>

#include <Common/Base/hkBase.h>
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastInput.h>
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastOutput.h>

class hkpWorld;
class hkVector4;


namespace bsRayCastUtil
{
	/*	Casts a ray from the origin to the destination in the specified world and get
		the closest hit.
	*/
	void castRay(const XMVECTOR& origin, const XMVECTOR& destination,
		hkpWorld* world, hkpWorldRayCastOutput& hitCollectorOut);

	/*	Calculates the destination based on origin, forward direction and ray length
		and calls the above overload of the function.
	*/
	void castRay(const XMVECTOR& origin, const XMVECTOR& forward, float rayLength,
		hkpWorld* world, hkpWorldRayCastOutput& hitCollectorOut, XMVECTOR& destinationOut);


	hkVector4 getHitPosition(const hkpWorldRayCastInput& input,
		const hkpWorldRayCastOutput& output);


	/*	Projects a screen point to object space. Returns the input point in object space.
		Note: bsCamera has a function which uses this function, but is easier to use.

		screenPoint: 3D vector describing the point on the screen, where x and y are within
		the pixel width and height of the screen, and z is the depth.
		screenSize: The size of the screen in pixels.
		projection: The projection matrix used when rendering.
		view: The camera's view matrix.
	*/
	XMVECTOR screenSpaceToObjectSpace(const XMVECTOR& screenPoint,
		const XMFLOAT2& screenSize, const XMMATRIX& projection, const XMMATRIX& view);
}
