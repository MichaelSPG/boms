#pragma once

#include <xnamath.h>

struct bsFrustum;


namespace bsCollision
{
__declspec(align(16)) struct Sphere
{
	/*	Returns the radius portion of the stored vector.
	*/
	inline float getRadius() const
	{
		return XMVectorGetW(positionAndRadius);
	}

	inline void setRadius(float radius)
	{
		positionAndRadius = XMVectorSetW(positionAndRadius, radius);
	}

	inline void setRadius(const XMVECTOR& radius)
	{
		positionAndRadius = XMVectorSelect(positionAndRadius, radius,
			XMVectorSelectControl(0, 0, 0, 1));
	}

	//This may contain position in local or world space depending on where it is used.
	//Position is in xyz, radius in w.
	XMVECTOR positionAndRadius;
};

Sphere createSphereFromPoints(const XMFLOAT3* points, unsigned int pointCount);

Sphere mergeSpheres(const Sphere& sphere1, const Sphere& sphere2);

enum IntersectionResult
{
	OUTSIDE,
	INTERSECTING,
	INSIDE,
};

/*	Calculates whether a sphere is intersecting with a frustum or not.
	The sphere's position component should be in local space, while spherePosition should
	be the sphere's world space offset.
	The frustum's planes should be in world space.

	Returns OUTSIDE, INTERSECTING or INSIDE.
*/
IntersectionResult intersectSphereFrustum(const Sphere& sphere,
	const XMVECTOR& spherePosition, const bsFrustum& frustum);

/*	Test if two spheres are intersecting.
	Returns OUTSIDE or INTERSECTING.
*/
IntersectionResult intersectSphereSphere(const Sphere& sphere1,
	const XMVECTOR& sphere1Position, const Sphere& sphere2, const XMVECTOR& sphere2Position);
}
