#ifndef BS_SHAPEPHANTOM_H
#define BS_SHAPEPHANTOM_H

#include "bsConfig.h"

#include <vector>

#include <Common/Base/hkBase.h>
#include <Physics/Dynamics/Phantom/hkpCachingShapePhantom.h>

class bsSceneNode;


class bsNodeCollectorPhantom : public hkpCachingShapePhantom//hkpSimpleShapePhantom
{
public:
	bsNodeCollectorPhantom(const hkpShape* shape, const hkTransform& transform)
		: hkpCachingShapePhantom(shape, transform)
	{}

	~bsNodeCollectorPhantom()
	{}

	//HK_PHANTOM_USER0
	inline virtual hkpPhantomType getType() const
	{
		return HK_PHANTOM_USER0;
	}

	virtual void addOverlappingCollidable(hkpCollidable* collidable);

	virtual void removeOverlappingCollidable(hkpCollidable* collidable);
	/*
	inline const std::vector<hkpCollidable*>& getOverlappingCollidables() const
	{
		return mCollidables;
	}
	*/
	std::vector<bsSceneNode*> getOverlappingSceneNodes();

private:
	//std::vector<hkpCollidable*>		mCollidables;
	std::vector<bsSceneNode*>		mSceneNodes;
};

#endif // BS_SHAPEPHANTOM_H