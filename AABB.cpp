#include "AABB.h"

#include "Node.h"

AABB::~AABB()
{

}

bool AABB::intersects(const AABB& aabb) const
{
	const XMFLOAT3& ownerPosition = mOwner->getPosition();
	const XMFLOAT3& otherPosition = aabb.mOwner->getPosition();

	//Try to prove the two AABBs aren't intersecting.
	if (otherPosition.x + aabb.mHalfExtents.x < ownerPosition.x - mHalfExtents.x)
		return false;
	if (otherPosition.x - aabb.mHalfExtents.x > ownerPosition.x + mHalfExtents.x)
		return false;

	if (otherPosition.y + aabb.mHalfExtents.y < ownerPosition.y - mHalfExtents.y)
		return false;
	if (otherPosition.y - aabb.mHalfExtents.y > ownerPosition.y + mHalfExtents.y)
		return false;

	if (otherPosition.z + aabb.mHalfExtents.z < ownerPosition.z - mHalfExtents.z)
		return false;
	if (otherPosition.z - aabb.mHalfExtents.z > ownerPosition.z + mHalfExtents.z)
		return false;

	//Must be intersecting.
	return true;
}
