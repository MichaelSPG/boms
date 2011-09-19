#pragma once

#include <Windows.h>
#include <xnamath.h>

class hkpWorld;
struct hkpWorldRayCastOutput;
struct hkpWorldRayCastInput;


namespace bsRayCastUtil
{
	/*	Casts a ray from the origin to the destination in the specified world.
		
		Reported hits depends on the type of hit collector used.
		A hkpWorldRayCastOutput reports only a single hit, while other collectors can
		report all hits, with filtering if desired.
	*/
	void castRay(const XMVECTOR& origin, const XMVECTOR& destination,
		hkpWorld* world, hkpWorldRayCastOutput& hitCollectorOut);

	void castRay(const XMVECTOR& origin, const XMVECTOR& rotation, float rayLength,
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
