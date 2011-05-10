#ifndef BS_SHAPEPHANTOM_H
#define BS_SHAPEPHANTOM_H

#include "bsConfig.h"

#include <vector>

#include <Common/Base/hkBase.h>
#include <Physics/Dynamics/Phantom/hkpCachingShapePhantom.h>

class bsSceneNode;


/*	Keeps track of overlapping phantoms which are created by scene nodes only.
	The scene nodes that are in the collection at any given time may only overlap in
	the broadphase (AABB vs AABB), so additional checks are done every frame to
	return a vector which only contains scene nodes which overlap after narrowphase
	checks.
*/
class bsNodeCollectorPhantom : public hkpCachingShapePhantom//hkpSimpleShapePhantom
{
public:
	bsNodeCollectorPhantom(const hkpShape* shape, const hkTransform& transform)
		: hkpCachingShapePhantom(shape, transform)
	{}

	~bsNodeCollectorPhantom()
	{}

	//Returns HK_PHANTOM_USER0
	inline virtual hkpPhantomType getType() const
	{
		return HK_PHANTOM_USER0;
	}

	virtual void addOverlappingCollidable(hkpCollidable* collidable);

	virtual void removeOverlappingCollidable(hkpCollidable* collidable);
	
	/*	Returns a new vector consisting of scene nodes that overlap this phantom.
		Performs narrowphase collision detection on all potential overlaps (AABB vs AABB
		overlaps).
	*/
	std::vector<bsSceneNode*> getOverlappingSceneNodes();

private:
	std::vector<bsSceneNode*>		mSceneNodes;
};

#endif // BS_SHAPEPHANTOM_H