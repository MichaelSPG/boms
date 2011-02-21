#include "AABB.h"

#include "Node.h"

AABB::~AABB()
{

}

const IntersectionResult AABB::intersects(const AABB& aabb) const
{
	const XMFLOAT3& ownerPosition = mOwner->getPosition();
	const XMFLOAT3& otherPosition = aabb.mOwner->getPosition();

	/*
	//Try to prove the two AABBs are not intersecting.
	if (  otherPosition.x + aabb.mHalfExtents.x < ownerPosition.x - mHalfExtents.x
		||otherPosition.x - aabb.mHalfExtents.x > ownerPosition.x + mHalfExtents.x

		||otherPosition.y + aabb.mHalfExtents.y < ownerPosition.y - mHalfExtents.y
		||otherPosition.y - aabb.mHalfExtents.y > ownerPosition.y + mHalfExtents.y

		||otherPosition.z + aabb.mHalfExtents.z < ownerPosition.z - mHalfExtents.z
		||otherPosition.z - aabb.mHalfExtents.z > ownerPosition.z + mHalfExtents.z)
	{
		return OUTSIDE;
	}
	*/
	//Must be at least partially intersecting at this point.

	/*
	if (otherPosition.x + aabb.mHalfExtents.x < ownerPosition.x - mHalfExtents.x)
	{
		if (otherPosition.x - aabb.mHalfExtents.x > ownerPosition.x + mHalfExtents.x)
		{
			if (otherPosition.y + aabb.mHalfExtents.y < ownerPosition.y - mHalfExtents.y)
			{
				if (otherPosition.y - aabb.mHalfExtents.y > ownerPosition.y + mHalfExtents.y)
				{
					if (otherPosition.z + aabb.mHalfExtents.z < ownerPosition.z - mHalfExtents.z)
					{
						if (otherPosition.z - aabb.mHalfExtents.z > ownerPosition.z + mHalfExtents.z)
						{
							return CONTAINED;
						}
						else
						{
							return INTERSECTING;
						}
					}
					else
					{
						return INTERSECTING;
					}
				}
				else
				{
					return INTERSECTING;
				}
			}
			else
			{
				return INTERSECTING;
			}
		}
		else
		{
			return INTERSECTING;
		}
	}
	else
	{
		return OUTSIDE;
	}
	*/

	int intersections = 0;

	if (otherPosition.x + aabb.mHalfExtents.x < ownerPosition.x - mHalfExtents.x)
	{
		++intersections;
	}
	if (otherPosition.x - aabb.mHalfExtents.x > ownerPosition.x + mHalfExtents.x)
	{
		++intersections;
	}

	if (otherPosition.y + aabb.mHalfExtents.y < ownerPosition.y - mHalfExtents.y)
	{
		++intersections;
	}
	if (otherPosition.y - aabb.mHalfExtents.y > ownerPosition.y + mHalfExtents.y)
	{
		++intersections;
	}

	if (otherPosition.z + aabb.mHalfExtents.z < ownerPosition.z - mHalfExtents.z)
	{
		++intersections;
	}
	if (otherPosition.z - aabb.mHalfExtents.z > ownerPosition.z + mHalfExtents.z)
	{
		++intersections;
	}


	if (intersections == 0)
	{
		return OUTSIDE;
	}
	else if (intersections < 6)
	{
		return INTERSECTING;
	}
	else
	{
		return CONTAINED;
	}
}
