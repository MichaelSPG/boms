#pragma once

#include <xnamath.h>

struct bsFrustum;


namespace bsCollision
{
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

}
