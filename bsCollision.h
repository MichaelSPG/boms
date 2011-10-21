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

enum IntersectionResult
{
	OUTSIDE,
	INTERSECTING,
	INSIDE,
	INTERSECTING_OR_INSIDE,
};

/*	Returns OUTSIDE, INTERSECTING or INSIDE.
*/
IntersectionResult intersectAxisAlignedBoxFrustum(const XMVECTOR& aabbExtents,
	const XMVECTOR& aabbPosition, const bsFrustum& frustum);

/*	Returns OUTSIDE or INTERSECTING_OR_INSIDE only.
	Only checks if the box is outside the frustum. If it is not outside, it can be either
	intersecting or inside the frustum.
*/
IntersectionResult intersectAxisAlignedBoxFrustumOutsideOnly(const XMVECTOR& aabbExtents,
	const XMVECTOR& aabbPosition, const bsFrustum& frustum);

IntersectionResult intersectSphereFrustum(const Sphere& sphere, const bsFrustum& frustum);

IntersectionResult intersectSphereSphere(const Sphere& sphere1, const Sphere& sphere2);
}
