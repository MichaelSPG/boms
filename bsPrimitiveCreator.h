#pragma once

#include <xnamath.h>

class bsSceneNode;
class bsMeshCache;


class bsPrimitiveCreator
{
public:
	bsPrimitiveCreator(bsMeshCache& meshCache)
		: mMeshCache(meshCache)
	{}

	~bsPrimitiveCreator()
	{}

	/*	Creates a node with a sphere mesh and rigid body with radius equal to input.
		The sphere's position will be at 0,0,0, and the rotation will be identity.
		Scale will be based on input parameter.
	*/
	bsSceneNode* createSphere(float radius);

	/*	Creates a node with a box mesh and rigid body with extents specified by the parameter.
		The box' position will be at 0,0,0, and the rotation will be identity.
		Scale will be based on input parameter.
	*/
	bsSceneNode* createBox(const XMVECTOR& halfExtents);

	/*	Creates a node with a plane mesh and rigid body with extents equal to input's x
		and z components.
		The plane's position will be at 0,0,0, and the rotation will be identity.
		Scale will be based on input parameter. y and w components are ignored.
		The plane's normal will be 0,1,0 (pointing straight up).
	*/
	bsSceneNode* createPlane(const XMVECTOR& halfExtents);

private:
	bsMeshCache& mMeshCache;
};
