#ifndef AABB_H
#define AABB_H

#include "Dx11Renderer.h"

class Node;

enum IntersectionResult
{
	//Completely outside
	OUTSIDE,

	//At least partially intersecting
	INTERSECTING,

	//Fits completely within the box
	CONTAINED
};


class AABB
{
public:
	friend class OctNode;
	friend class SceneNode;
	friend class SceneGraph;

	

	AABB(float halfExtentsX, float halfExtentsY, float halfExtentsZ, Node* owner)
		: mHalfExtents(halfExtentsX, halfExtentsY, halfExtentsZ)
		, mOwner(owner)
	{}

	AABB(const XMFLOAT3& halfExtents, Node* owner)
		: mHalfExtents(halfExtents)
		, mOwner(owner)
	{}

	AABB()
		: mHalfExtents(0.0f, 0.0f, 0.0f)
		, mOwner(nullptr)
	{}

	AABB(const AABB& aabb, Node* owner)
		: mHalfExtents(aabb.mHalfExtents)
		, mOwner(owner)
	{}

	~AABB();

	const IntersectionResult intersects(const AABB& aabb) const;

	inline Node* getOwner() const
	{
		return mOwner;
	}
	
	inline const XMFLOAT3& getHalfExtents() const
	{
		return mHalfExtents;
	}
	
private:
	XMFLOAT3	mHalfExtents;
	Node*		mOwner;
};

#endif // AABB