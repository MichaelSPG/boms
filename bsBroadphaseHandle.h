#ifndef BS_BROADPHASEHANDLE_H
#define BS_BROADPHASEHANDLE_H

#include "bsConfig.h"

#include <Physics/Collide/BroadPhase/hkpBroadPhaseHandle.h>

class bsSceneNode;


class bsBroadphaseHandle : public hkpBroadPhaseHandle
{
public:
	bsBroadphaseHandle(bsSceneNode* sceneNode)
		: mSceneNode(sceneNode)
	{}

	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CDINFO, bsBroadphaseHandle);

	~bsBroadphaseHandle()
	{}

	bsSceneNode* getSceneNode() const
	{
		return mSceneNode;
	}
	

private:
	bsSceneNode*	mSceneNode;
};

#endif // BS_BROADPHASEHANDLE_H
