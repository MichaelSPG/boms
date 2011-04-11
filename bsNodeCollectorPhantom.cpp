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

		//User data will only ever be 0 (default) or a point to the scene node it
		//belongs to, so this will always be safe.
		if (userData)
		{
			mSceneNodes.push_back(reinterpret_cast<bsSceneNode*>(userData));
			hkpCachingShapePhantom::addOverlappingCollidable(collidable);
		}
		/*
		if (phantom->getType() == HK_PHANTOM_AABB)
		{
			const hkpAabbPhantom* aabbPhantom = static_cast<const hkpAabbPhantom*>(phantom);
			const hkUlong userData = aabbPhantom->getUserData();

			//User data will only ever be 0 (default) or a point to the scene node it
			//belongs to, so this will always be safe.
			if (userData)
			{
				mSceneNodes.push_back(reinterpret_cast<bsSceneNode*>(userData));
			}
		}*/
	}

	//hkpCachingShapePhantom::addOverlappingCollidable(collidable);
//	mCollidables.push_back(collidable);
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
		/*
		if (phantom->getType() == HK_PHANTOM_AABB)
		{
			const hkpAabbPhantom* aabbPhantom = static_cast<const hkpAabbPhantom*>(phantom);
			const hkUlong userData = aabbPhantom->getUserData();

			if (userData)
			{
				const bsSceneNode* node = reinterpret_cast<bsSceneNode*>(userData);

				for (unsigned int i = 0; i < mSceneNodes.size(); ++i)
				{
					if (node == mSceneNodes[i])
					{
						bsT::unordered_erase(mSceneNodes, mSceneNodes[i]);

						break;
					}
				}
			}
		}*/
	}
	//hkpCachingShapePhantom::removeOverlappingCollidable(collidable);
	/*
	for (unsigned int i = 0; i < mCollidables.size(); ++i)
	{
		if (mCollidables[i] == collidable)
		{
			bsT::unordered_erase(mCollidables, mCollidables[i]);

			break;
		}
	}*/
}

std::vector<bsSceneNode*> bsNodeCollectorPhantom::getOverlappingSceneNodes()
{
	hkpAllCdBodyPairCollector collector;
	getPenetrations(collector);

	const hkArray<hkpRootCdBodyPair>& hits = collector.getHits();
	if (hits.isEmpty())
	{
		return std::vector<bsSceneNode*>(0);
	}

	std::vector<bsSceneNode*> currentOverlappingNodes;

	for (unsigned int i = 0, count = hits.getSize(); i < count; ++i)
	{
		const hkpCollidable* curentHit = hits[i].m_rootCollidableB;

		if (curentHit->getType() == hkpWorldObject::BROAD_PHASE_PHANTOM)
		{
			const hkpPhantom* phantom = static_cast<const hkpPhantom*>(curentHit->getOwner());
			const hkUlong userData = phantom->getUserData();

			if (userData)
			{
				currentOverlappingNodes.push_back(reinterpret_cast<bsSceneNode*>(userData));
			}
		}
	}

	return currentOverlappingNodes;
}
