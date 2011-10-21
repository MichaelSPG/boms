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

	// outside the plane?
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

	// outside the plane?
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

//-----------------------------------------------------------------------------
// Return TRUE if the quaterion is a unit quaternion.
// Taken from XNA Collision.
//-----------------------------------------------------------------------------
inline BOOL XMQuaternionIsUnit(FXMVECTOR Q, float epsilon = 0.0001f)
{
	const XMVECTOR difference = XMVector4Length(Q) - XMVectorSplatOne();

	return XMVector4Less(XMVectorAbs(difference), XMVectorReplicate(epsilon));
}

//-----------------------------------------------------------------------------
// Return the point on the line segement (S1, S2) nearest the point P.
// Taken from XNA Collision.
//-----------------------------------------------------------------------------
inline XMVECTOR pointOnLineSegmentNearestPoint(FXMVECTOR S1, FXMVECTOR S2, FXMVECTOR P)
{
	const XMVECTOR dir = S2 - S1;
	const XMVECTOR projection = XMVectorSubtract(XMVector3Dot(P, dir), XMVector3Dot(S1, dir));
	const XMVECTOR lengthSq = XMVector3Dot(dir, dir);

	const XMVECTOR t = XMVectorMultiply(projection, XMVectorReciprocal(lengthSq));
	XMVECTOR point = S1 + t * dir;

	// t < 0
	const XMVECTOR selectS1 = XMVectorLess(projection, XMVectorZero());
	point = XMVectorSelect(point, S1, selectS1);

	// t > 1
	const XMVECTOR selectS2 = XMVectorGreater(projection, lengthSq);
	point = XMVectorSelect(point, S2, selectS2);

	return point;
}

// Return values: 0 = no intersection, 
//                1 = intersection, 
//                2 = A is completely inside B
bsCollision::IntersectionResult bsCollision::intersectSphereFrustum(const Sphere& sphere,
	const bsFrustum& frustum)
{
	const XMVECTOR zero = XMVectorZero();

	// Build the frustum planes.
	const XMVECTOR* frustumPlanes = frustum.planes;

	// Normalize the planes so we can compare to the sphere radius.
	//frustumPlanes[2] = XMVector3Normalize(frustumPlanes[2]);
	//frustumPlanes[3] = XMVector3Normalize(frustumPlanes[3]);
	//frustumPlanes[4] = XMVector3Normalize(frustumPlanes[4]);
	//frustumPlanes[5] = XMVector3Normalize(frustumPlanes[5]);

	// Load origin and orientation of the frustum.
	const XMVECTOR& frustumPosition = frustum.position;
	const XMVECTOR frustumRotation = frustum.rotation;

	XMASSERT(XMQuaternionIsUnit(frustumRotation));

	// Load the sphere.
	XMVECTOR sphereCenter = sphere.positionAndRadius;
	const XMVECTOR sphereRadius = XMVectorReplicate(sphere.getRadius());

	// Transform the center of the sphere into the local space of frustum.
	sphereCenter = XMVector3InverseRotate(XMVectorSubtract(sphereCenter, frustumPosition),
		frustumRotation);

	// Set w of the center to one so we can dot4 with the plane.
	sphereCenter = XMVectorInsert(sphereCenter, XMVectorSplatOne(), 0, 0, 0, 0, 1);

	// Check against each plane of the frustum.
	XMVECTOR outside = XMVectorFalseInt();
	XMVECTOR insideAll = XMVectorTrueInt();
	XMVECTOR centerInsideAll = XMVectorTrueInt();

	XMVECTOR dist[6];

	for (unsigned int i = 0; i < 6; ++i)
	{
		dist[i] = XMVector4Dot(sphereCenter, frustumPlanes[i]);

		// outside the plane?
		outside = XMVectorOrInt(outside, XMVectorGreater(dist[i], sphereRadius));

		// Fully inside the plane?
		insideAll = XMVectorAndInt(insideAll, XMVectorLessOrEqual(dist[i], -sphereRadius));

		// Check if the center is inside the plane.
		centerInsideAll = XMVectorAndInt(centerInsideAll, XMVectorLessOrEqual(dist[i], zero));
	}

	// If the sphere is outside any of the planes it is outside. 
	if (XMVector4EqualInt(outside, XMVectorTrueInt()))
	{
		return OUTSIDE;
	}

	// If the sphere is inside all planes it is fully inside.
	if (XMVector4EqualInt(insideAll, XMVectorTrueInt()))
	{
		return INSIDE;
	}

	// If the center of the sphere is inside all planes and the sphere intersects 
	// one or more planes then it must intersect.
	if (XMVector4EqualInt(centerInsideAll, XMVectorTrueInt()))
	{
		return INTERSECTING;
	}

	// The sphere may be outside the frustum or intersecting the frustum.
	// Find the nearest feature (face, edge, or corner) on the frustum 
	// to the sphere.

	// The faces adjacent to each face are:
	static const INT adjacent_faces[6][4] =
	{
		{ 2, 3, 4, 5 },    // 0
		{ 2, 3, 4, 5 },    // 1
		{ 0, 1, 4, 5 },    // 2
		{ 0, 1, 4, 5 },    // 3
		{ 0, 1, 2, 3 },    // 4
		{ 0, 1, 2, 3 }
	};  // 5

	XMVECTOR intersects = XMVectorFalseInt();

	// Check to see if the nearest feature is one of the planes.
	for (unsigned int i = 0; i < 6; ++i)
	{
		// Find the nearest point on the plane to the center of the sphere.
		XMVECTOR point = XMVectorSubtract(sphereCenter,
			XMVectorMultiply(frustumPlanes[i], dist[i]));

		// Set w of the point to one.
		point = XMVectorInsert(point, XMVectorSplatOne(), 0, 0, 0, 0, 1);

		// If the point is inside the face (inside the adjacent planes) then
		// this plane is the nearest feature.
		XMVECTOR insideFace = XMVectorTrueInt();

		for (unsigned int j = 0; j < 4; ++j)
		{
			int plane_index = adjacent_faces[i][j];

			insideFace = XMVectorAndInt(insideFace,
				XMVectorLessOrEqual(XMVector4Dot(point, frustumPlanes[plane_index]), zero));
		}

		// Since we have already checked distance from the plane we know that the
		// sphere must intersect if this plane is the nearest feature.
		intersects = XMVectorOrInt(intersects, 
			XMVectorAndInt(XMVectorGreater(dist[i], zero), insideFace));
	}

	if (XMVector4EqualInt(intersects, XMVectorTrueInt()))
	{
		return INTERSECTING;
	}

	// Build the corners of the frustum.
	const XMVECTOR rightTop = XMVectorSet(frustum.rightSlope, frustum.topSlope, 1.0f, 0.0f);
	const XMVECTOR rightBottom = XMVectorSet(frustum.rightSlope, frustum.bottomSlope, 1.0f, 0.0f);
	const XMVECTOR leftTop = XMVectorSet(frustum.leftSlope, frustum.topSlope, 1.0f, 0.0f);
	const XMVECTOR leftBottom = XMVectorSet(frustum.leftSlope, frustum.bottomSlope, 1.0f, 0.0f);
	const XMVECTOR nearClip = XMVectorReplicate(frustum.nearClip);
	const XMVECTOR farClip = XMVectorReplicate(frustum.farClip);

	XMVECTOR corners[8];
	corners[0] = XMVectorMultiply(rightTop, nearClip);
	corners[1] = XMVectorMultiply(rightBottom, nearClip);
	corners[2] = XMVectorMultiply(leftTop, nearClip);
	corners[3] = XMVectorMultiply(leftBottom, nearClip);
	corners[4] = XMVectorMultiply(rightTop, farClip);
	corners[5] = XMVectorMultiply(rightBottom, farClip);
	corners[6] = XMVectorMultiply(leftTop, farClip);
	corners[7] = XMVectorMultiply(leftBottom, farClip);

	// The Edges are:
	static const int edges[12][2] =
	{
		{ 0, 1 }, { 2, 3 }, { 0, 2 }, { 1, 3 },    // nearClip plane
		{ 4, 5 }, { 6, 7 }, { 4, 6 }, { 5, 7 },    // farClip plane
		{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
	}; // nearClip to farClip

	const XMVECTOR radiusSq = sphereRadius * sphereRadius;

	// Check to see if the nearest feature is one of the edges (or corners).
	for (unsigned int i = 0; i < 12; ++i)
	{
		const int ei0 = edges[i][0];
		const int ei1 = edges[i][1];

		// Find the nearest point on the edge to the center of the sphere.
		// The corners of the frustum are included as the endpoints of the edges.
		const XMVECTOR point = pointOnLineSegmentNearestPoint(corners[ei0], corners[ei1],
			sphereCenter);

		const XMVECTOR delta = sphereCenter - point;

		const XMVECTOR distSq = XMVector3Dot(delta, delta );

		// If the distance to the center of the sphere to the point is less than 
		// the radius of the sphere then it must intersect.
		intersects = XMVectorOrInt(intersects, XMVectorLessOrEqual(distSq, radiusSq));
	}

	if (XMVector4EqualInt(intersects, XMVectorTrueInt()))
	{
		return INTERSECTING;
	}

	// The sphere must be outside the frustum.
	return OUTSIDE;
}

bsCollision::IntersectionResult bsCollision::intersectSphereSphere(const Sphere& sphere1,
	const Sphere& sphere2)
{
	const XMVECTOR& center1 = sphere1.positionAndRadius;
	const XMVECTOR radius1 = XMVectorReplicate(sphere1.getRadius());

	const XMVECTOR& center2 = sphere2.positionAndRadius;
	const XMVECTOR radius2 = XMVectorReplicate(sphere2.getRadius());

	//Distance squared between centers.
	const XMVECTOR delta = XMVectorSubtract(center2, center1);
	const XMVECTOR distanceSquared = XMVector3LengthSq(delta);

	//Sum of radii squared.
	XMVECTOR radiusSquared = XMVectorAdd(radius1, radius2);
	radiusSquared = XMVectorMultiply(radiusSquared, radiusSquared);

	return XMVector3LessOrEqual(distanceSquared, radiusSquared) ? INTERSECTING : OUTSIDE;
}
