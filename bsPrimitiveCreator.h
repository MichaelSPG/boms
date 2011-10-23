#pragma once

#include <xnamath.h>

class bsEntity;
class bsMeshCache;


class bsPrimitiveCreator
{
public:
	bsPrimitiveCreator(bsMeshCache& meshCache)
		: mMeshCache(meshCache)
	{}

	bsPrimitiveCreator(const bsPrimitiveCreator& other)
		: mMeshCache(other.mMeshCache)
	{}

	/*	Creates an entity with a sphere mesh and rigid body with radius equal to input.
		The sphere's position will be at 0,0,0, and the rotation will be identity.
		Scale will be based on input parameter.
	*/
	bsEntity* createSphere(float radius);

	/*	Creates an entity with a box mesh and rigid body with extents specified by the parameter.
		The box' position will be at 0,0,0, and the rotation will be identity.
		Scale will be based on input parameter.
	*/
	bsEntity* createBox(const XMVECTOR& halfExtents);

	/*	Creates an entity with a plane mesh and rigid body with extents equal to input's x
		and z components.
		The plane's position will be at 0,0,0, and the rotation will be identity.
		Scale will be based on input parameter. y and w components are ignored.
		The plane's normal will be 0,1,0 (pointing straight up).
	*/
	bsEntity* createPlane(const XMVECTOR& halfExtents);

private:
	bsPrimitiveCreator& operator=(const bsPrimitiveCreator&);

	bsMeshCache& mMeshCache;
};
