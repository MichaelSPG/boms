#include "bsNodeCollectorPhantom.h"

#include <algorithm>
#include <cassert>

#include "bsTemplates.h"
//#include "bsSceneNode.h"
#include <Physics\Collide\Query\Collector\BodyPairCollector\hkpAllCdBodyPairCollector.h>
#include <Physics/Collide/Shape/Convex/ConvexVertices/hkpConvexVerticesShape.h>


void bsNodeCollectorPhantom::addOverlappingCollidable(hkpCollidable* collidable)
{
	//Only phantoms in the graphics world will be interesting to keep track of.
	if (collidable->getType() == hkpWorldObject::BROAD_PHASE_PHANTOM)
	{
		const hkpPhantom* phantom = static_cast<const hkpPhantom*>(collidable->getOwner());
		const hkUlong userData = phantom->getUserData();

		//User data will only ever be 0 (default) or a pointer to the scene node it
		//belongs to, so this will always be safe.
		if (userData)
		{
			mSceneNodes.push_back(reinterpret_cast<bsSceneNode*>(userData));
			hkpCachingShapePhantom::addOverlappingCollidable(collidable);
		}
	}
}

void bsNodeCollectorPhantom::removeOverlappingCollidable(hkpCollidable* collidable)
{
	if (collidable->getType() == hkpWorldObject::BROAD_PHASE_PHANTOM)
	{
		const hkpPhantom* phantom = static_cast<hkpPhantom*>(collidable->getOwner());
		const hkUlong userData = phantom->getUserData();

		if (userData)
		{
			const bsSceneNode* node = reinterpret_cast<bsSceneNode*>(userData);

			//Find the match in the current collection
			for (unsigned int i = 0, count = mSceneNodes.size(); i < count; ++i)
			{
				if (node == mSceneNodes[i])
				{
					bsT::unordered_erase(mSceneNodes, mSceneNodes[i]);

					hkpCachingShapePhantom::removeOverlappingCollidable(collidable);

					break;
				}
			}
		}
	}
}

std::vector<bsSceneNode*> bsNodeCollectorPhantom::getOverlappingSceneNodes()
{
	//This is where the narrowphase collision detection happens, allowing the function to
	//return only the scene nodes that are most likely to be within the frustum.

	hkpAllCdBodyPairCollector collector;
	getPenetrations(collector);

	const hkArray<hkpRootCdBodyPair>& hits = collector.getHits();
	const int hitCount = hits.getSize();
	if (!hitCount)
	{
		//Return an empty vector if there are no narrowphase hits
		return std::vector<bsSceneNode*>(0);
	}
	
	std::vector<bsSceneNode*> currentOverlappingNodes(hitCount);

	for (int i = 0; i < hitCount; ++i)
	{
		const hkpCollidable* curentHit = hits[i].m_rootCollidableB;

		//This will only ever happen if addOverlappingCollidable accepts a non-phantom,
		//so hopefully never.
		assert(curentHit->getType() == hkpWorldObject::BROAD_PHASE_PHANTOM);

		const hkpPhantom* phantom = static_cast<const hkpPhantom*>(curentHit->getOwner());
		
		//Only phantoms with user data (reinterpret_cast'd 'this' by owning scene node)
		//should pass through addOverlappingCollidable.
		assert(phantom->getUserData());

		currentOverlappingNodes[i] = reinterpret_cast<bsSceneNode*>(phantom->getUserData());
	}

	return currentOverlappingNodes;
}
