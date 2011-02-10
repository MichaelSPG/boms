#ifndef AABB_H
#define AABB_H

#include "Dx11Renderer.h"

class Node;

class AABB
{
public:
	friend class OctNode;
	friend class SceneNode;
	friend class SceneGraph;

public:
	AABB(float halfExtentsX, float halfExtentsY, float halfExtentsZ, Node* owner)
		: mHalfExtents(halfExtentsX, halfExtentsY, halfExtentsZ)
		, mOwner(owner)
	{}

	AABB(const XMFLOAT3& halfExtents, Node* owner)
		: mHalfExtents(halfExtents)
		, mOwner(owner)
	{}

	~AABB();

	bool intersects(const AABB& aabb) const;

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