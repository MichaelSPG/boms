#include "StdAfx.h"

#include "bsCollision.h"
#include "bsFrustum.h"


/*	Checks if the box is outside or inside the frustum.
*/
inline void fastIntersectAxisAlignedBoxPlane(const XMVECTOR& center, const XMVECTOR& extents,
	const XMVECTOR& plane, XMVECTOR& outside, XMVECTOR& inside)
{
	// Compute the distance to the center of the box.
	const XMVECTOR distance = XMVector4Dot(center, plane);

	// Project the axes of the box onto the normal of the plane.  Half the
	// length of the projection (sometime called the "radius") is equal to
	// h(u) * abs(n dot b(u))) + h(v) * abs(n dot b(v)) + h(w) * abs(n dot b(w))
	// where h(i) are extents of the box, n is the plane normal, and b(i) are the 
	// axes of the box. In this case b(i) = [(1,0,0), (0,1,0), (0,0,1)].
	const XMVECTOR radius = XMVector3Dot(extents, XMVectorAbs(plane));

	// Outside the plane?
	outside = XMVectorGreater(distance, radius);

	// Fully inside the plane?
	inside = XMVectorLess(distance, -radius);
}

/*	Checks if the box is outside the frustum only.
	Skips checking if the box is inside, so a box that is not outside may be either inside
	or intersecting.
*/
inline void fastIntersectAxisAlignedBoxPlaneOutsideOnly(const XMVECTOR& center,
	const XMVECTOR& extents, const XMVECTOR& plane, XMVECTOR& outside)
{
	// Compute the distance to the center of the box.
	const XMVECTOR distance = XMVector4Dot(center, plane);

	// Project the axes of the box onto the normal of the plane.  Half the
	// length of the projection (sometime called the "radius") is equal to
	// h(u) * abs(n dot b(u))) + h(v) * abs(n dot b(v)) + h(w) * abs(n dot b(w))
	// where h(i) are extents of the box, n is the plane normal, and b(i) are the 
	// axes of the box. In this case b(i) = [(1,0,0), (0,1,0), (0,0,1)].
	const XMVECTOR radius = XMVector3Dot(extents, XMVectorAbs(plane));

	// Outside the plane?
	outside = XMVectorGreater(distance, radius);
}

bsCollision::IntersectionResult bsCollision::intersectAxisAlignedBoxFrustum(
	const XMVECTOR& aabbExtents, const XMVECTOR& aabbPosition, const bsFrustum& frustum)
{
	// Set w of the center to one so we can dot4 with a plane.
	const XMVECTOR center = XMVectorInsert(aabbPosition, XMVectorSplatOne(), 0, 0, 0, 0, 1);

	XMVECTOR outside, inside, anyOutside, allInside;

	fastIntersectAxisAlignedBoxPlane(center, aabbExtents, frustum.planes[0], outside, inside);
	anyOutside = outside;
	allInside = inside;

	fastIntersectAxisAlignedBoxPlane(center, aabbExtents, frustum.planes[1], outside, inside);
	anyOutside = XMVectorOrInt(anyOutside, outside);
	allInside = XMVectorAndInt(allInside, inside);

	fastIntersectAxisAlignedBoxPlane(center, aabbExtents, frustum.planes[2], outside, inside);
	anyOutside = XMVectorOrInt(anyOutside, outside);
	allInside = XMVectorAndInt(allInside, inside);

	fastIntersectAxisAlignedBoxPlane(center, aabbExtents, frustum.planes[3], outside, inside);
	anyOutside = XMVectorOrInt(anyOutside, outside);
	allInside = XMVectorAndInt(allInside, inside);

	fastIntersectAxisAlignedBoxPlane(center, aabbExtents, frustum.planes[4], outside, inside);
	anyOutside = XMVectorOrInt(anyOutside, outside);
	allInside = XMVectorAndInt(allInside, inside);

	fastIntersectAxisAlignedBoxPlane(center, aabbExtents, frustum.planes[5], outside, inside);
	anyOutside = XMVectorOrInt(anyOutside, outside);
	allInside = XMVectorAndInt(allInside, inside);

	if (XMVector4EqualInt(anyOutside, XMVectorTrueInt()))
	{
		return OUTSIDE;
	}

	if (XMVector4EqualInt(allInside, XMVectorTrueInt()))
	{
		return INSIDE;
	}

	return INTERSECTING;
}

bsCollision::IntersectionResult bsCollision::intersectAxisAlignedBoxFrustumOutsideOnly(
	const XMVECTOR& aabbExtents, const XMVECTOR& aabbPosition, const bsFrustum& frustum)
{
	// Set w of the center to one so we can dot4 with a plane.
	const XMVECTOR center = XMVectorInsert(aabbPosition, XMVectorSplatOne(), 0, 0, 0, 0, 1);

	XMVECTOR outside, anyOutside;

	fastIntersectAxisAlignedBoxPlaneOutsideOnly(center, aabbExtents, frustum.planes[0], outside);
	anyOutside = outside;

	fastIntersectAxisAlignedBoxPlaneOutsideOnly(center, aabbExtents, frustum.planes[1], outside);
	anyOutside = XMVectorOrInt(anyOutside, outside);

	fastIntersectAxisAlignedBoxPlaneOutsideOnly(center, aabbExtents, frustum.planes[2], outside);
	anyOutside = XMVectorOrInt(anyOutside, outside);

	fastIntersectAxisAlignedBoxPlaneOutsideOnly(center, aabbExtents, frustum.planes[3], outside);
	anyOutside = XMVectorOrInt(anyOutside, outside);

	fastIntersectAxisAlignedBoxPlaneOutsideOnly(center, aabbExtents, frustum.planes[4], outside);
	anyOutside = XMVectorOrInt(anyOutside, outside);

	fastIntersectAxisAlignedBoxPlaneOutsideOnly(center, aabbExtents, frustum.planes[5], outside);
	anyOutside = XMVectorOrInt(anyOutside, outside);

	if (XMVector4EqualInt(anyOutside, XMVectorTrueInt()))
	{
		return OUTSIDE;
	}

	return INTERSECTING_OR_INSIDE;
}
