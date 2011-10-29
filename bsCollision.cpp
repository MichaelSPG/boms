#include "StdAfx.h"

#include "bsCollision.h"
#include "bsFrustum.h"
#include "bsAssert.h"


bsCollision::Sphere bsCollision::createSphereFromPoints(const XMFLOAT3* points,
	unsigned int pointCount)
{
	BS_ASSERT2(pointCount > 0, "Cannot create a bounding sphere from 0 points");
	BS_ASSERT2(points, "Points array cannot not be null");


	//Taken from XNA::ComputeBoundingSphereFromPoints

	// Find the points with minimum and maximum x, y, and z
	XMVECTOR minX, maxX, minY, maxY, minZ, maxZ;

	minX = maxX = minY = maxY = minZ = maxZ = XMLoadFloat3(points);

	for (unsigned int i = 1; i < pointCount; ++i)
	{
		const XMVECTOR point = XMLoadFloat3(points + i);
		
		const float px = XMVectorGetX(point);
		const float py = XMVectorGetY(point);
		const float pz = XMVectorGetZ(point);

		if (px < XMVectorGetX(minX))
			minX = point;

		if (px > XMVectorGetX(maxX))
			maxX = point;

		if (py < XMVectorGetY(minY))
			minY = point;

		if (py > XMVectorGetY(maxY))
			maxY = point;

		if (pz < XMVectorGetZ(minZ))
			minZ = point;

		if (pz > XMVectorGetZ(maxZ))
			maxZ = point;
	}

	// Use the min/max pair that are farthest apart to form the initial sphere.
	const XMVECTOR deltaX = XMVectorSubtract(maxX, minX);
	const XMVECTOR distX = XMVector3Length(deltaX);

	const XMVECTOR deltaY = XMVectorSubtract(maxY, minY);
	const XMVECTOR distY = XMVector3Length(deltaY);

	const XMVECTOR deltaZ = XMVectorSubtract(maxZ, minZ);
	const XMVECTOR distZ = XMVector3Length(deltaZ);

	XMVECTOR center;
	XMVECTOR radius;

	if (XMVector3Greater(distX, distY))
	{
		if (XMVector3Greater(distX, distZ))
		{
			// Use min/max x.
			center = XMVectorScale(XMVectorAdd(maxX, minX), 0.5f);
			radius = XMVectorScale(distX, 0.5f);
		}
		else
		{
			// Use min/max z.
			center = XMVectorScale(XMVectorAdd(maxZ, minZ), 0.5f);
			radius = XMVectorScale(distZ, 0.5f);
		}
	}
	else // Y >= X
	{
		if (XMVector3Greater(distY, distZ))
		{
			// Use min/max y.
			center = XMVectorScale(XMVectorAdd(maxY, minY), 0.5f);
			radius = XMVectorScale(distY, 0.5f);
		}
		else
		{
			// Use min/max z.
			center = XMVectorScale(XMVectorAdd(maxZ, minZ), 0.5f);
			radius = XMVectorScale(distZ, 0.5f);
		}
	}

	// Add any points not inside the sphere.
	for (unsigned int i = 0; i < pointCount; ++i)
	{
		const XMVECTOR point = XMLoadFloat3(points + i);

		const XMVECTOR delta = XMVectorSubtract(point, center);

		const XMVECTOR dist = XMVector3Length(delta);

		if (XMVector3Greater(dist, radius))
		{
			// Adjust sphere to include the new point.
			radius = XMVectorScale(XMVectorAdd(radius, dist), 0.5f);

			center = XMVectorAdd(center, XMVectorMultiply((XMVectorSubtract(XMVectorReplicate(1.0f),
				XMVectorMultiply(radius, XMVectorReciprocal(dist)))), delta));
		}
	}

	Sphere boundingSphere;
	boundingSphere.positionAndRadius = center;
	boundingSphere.setRadius(XMVectorGetX(radius));

	return boundingSphere;
}

bsCollision::Sphere bsCollision::mergeSpheres(const Sphere& sphere1, const Sphere& sphere2)
{
	const XMVECTOR cenDiff = XMVectorSubtract(sphere2.positionAndRadius, sphere1.positionAndRadius);
	//float lenSqr = cenDiff.SquaredLength();
	//float rDiff = sphere1.radius - sphere0.radius;
	//float rDiffSqr = rDiff*rDiff;

	const float sphere1Radius = sphere1.getRadius();
	const float sphere2Radius = sphere2.getRadius();

	const float lenSqr = XMVectorGetX(XMVector3LengthSq(cenDiff));
	const float rDiff = sphere2Radius - sphere1Radius;
	const float rDiffSqr = rDiff * rDiff;


	if (rDiffSqr >= lenSqr)
	{
		return (rDiff >= 0.0f ? sphere2 : sphere1);
	}

	float length = sqrt(lenSqr);
	Sphere sphere;

	const float epsilon = 0.000001f;
	if (length > epsilon)
	{
		float coeff = (length + rDiff) / (2.0f * length);
		sphere.positionAndRadius = XMVectorAdd(sphere1.positionAndRadius,
			XMVectorScale(cenDiff, coeff));
	}
	else
	{
		sphere.positionAndRadius = sphere1.positionAndRadius;
	}

	sphere.setRadius(0.5f * (length + sphere1Radius + sphere2Radius));

	return sphere;
}

inline void fastIntersectSpherePlane(FXMVECTOR center, FXMVECTOR radius, FXMVECTOR plane,
	XMVECTOR& outside, XMVECTOR& inside)
{
	const XMVECTOR dist = XMVector4Dot(center, plane);

	// outside the plane?
	outside = XMVectorGreater(dist, radius);

	// Fully inside the plane?
	inside = XMVectorLess(dist, XMVectorScale(radius, -1.0f));
}

bsCollision::IntersectionResult bsCollision::intersectSphereFrustum(const Sphere& sphere,
	const XMVECTOR& spherePosition, const bsFrustum& frustum)
{
	//Taken from XNA::IntersectSphere6Planes

	XMVECTOR center = XMVectorAdd(sphere.positionAndRadius, spherePosition);
	const XMVECTOR radius = XMVectorReplicate(sphere.getRadius());

	const XMVECTOR* planes = frustum.planes;

	// Set w of the center to one so we can dot4 with a plane.
	center = XMVectorInsert(center, XMVectorSplatOne(), 0, 0, 0, 0, 1);

	XMVECTOR outside, inside;

	// Test against each plane.
	fastIntersectSpherePlane(center, radius, planes[0], outside, inside);

	XMVECTOR anyOutside = outside;
	XMVECTOR allInside = inside;

	fastIntersectSpherePlane(center, radius, planes[1], outside, inside);
	anyOutside = XMVectorOrInt(anyOutside, outside);
	allInside = XMVectorAndInt(allInside, inside);

	fastIntersectSpherePlane(center, radius, planes[2], outside, inside);
	anyOutside = XMVectorOrInt(anyOutside, outside);
	allInside = XMVectorAndInt(allInside, inside);

	fastIntersectSpherePlane(center, radius, planes[3], outside, inside);
	anyOutside = XMVectorOrInt(anyOutside, outside);
	allInside = XMVectorAndInt(allInside, inside);

	fastIntersectSpherePlane(center, radius, planes[4], outside, inside);
	anyOutside = XMVectorOrInt(anyOutside, outside);
	allInside = XMVectorAndInt(allInside, inside);

	fastIntersectSpherePlane(center, radius, planes[5], outside, inside);
	anyOutside = XMVectorOrInt(anyOutside, outside);
	allInside = XMVectorAndInt(allInside, inside);

	// If the sphere is outside any plane it is outside.
	if (XMVector4EqualInt(anyOutside, XMVectorTrueInt()))
	{
		return bsCollision::OUTSIDE;
	}

	//If the sphere is inside all planes it is inside.
	if (XMVector4EqualInt( allInside, XMVectorTrueInt()))
	{
		return bsCollision::INSIDE;
	}
	
	// The sphere is not inside all planes or outside a plane, it may intersect.
	return bsCollision::INTERSECTING;
}

bsCollision::IntersectionResult bsCollision::intersectSphereSphere(const Sphere& sphere1,
	const XMVECTOR& sphere1Position, const Sphere& sphere2, const XMVECTOR& sphere2Position)
{
	const XMVECTOR center1 = XMVectorAdd(sphere1.positionAndRadius, sphere1Position);
	const XMVECTOR radius1 = XMVectorReplicate(sphere1.getRadius());

	const XMVECTOR center2 = XMVectorAdd(sphere2.positionAndRadius, sphere1Position);
	const XMVECTOR radius2 = XMVectorReplicate(sphere2.getRadius());

	//Distance squared between centers.
	const XMVECTOR delta = XMVectorSubtract(center2, center1);
	const XMVECTOR distanceSquared = XMVector3LengthSq(delta);

	//Sum of radii squared.
	XMVECTOR radiusSquared = XMVectorAdd(radius1, radius2);
	radiusSquared = XMVectorMultiply(radiusSquared, radiusSquared);

	return XMVector3LessOrEqual(distanceSquared, radiusSquared) ? INTERSECTING : OUTSIDE;
}
