#pragma once

#include <xnamath.h>

class bsEntity;
class bsMeshCache;
class bsTextureCache;


/*	Utility class for making simple geometric objects with both graphical and physical
	components.
	
	All the objects created by this class are dynamic by default unless stated otherwise
	by comments for a specific function.
*/
class bsPrimitiveCreator
{
public:
	bsPrimitiveCreator(bsMeshCache& meshCache, bsTextureCache& textureCache)
		: mMeshCache(meshCache)
		, mTextureCache(textureCache)
	{}

	bsPrimitiveCreator(const bsPrimitiveCreator& other)
		: mMeshCache(other.mMeshCache)
		, mTextureCache(other.mTextureCache)
	{}

	/*	Creates an entity with a sphere mesh and rigid body with radius equal to input.
		The sphere's position will be at 0,0,0, and the rotation will be identity.
		Scale will be based on input parameter.
	*/
	bsEntity* createSphere(float radius) const;

	/*	Creates an entity with a box mesh and rigid body with extents specified by the parameter.
		The box' position will be at 0,0,0, and the rotation will be identity.
		Scale will be based on input parameter.
	*/
	bsEntity* createBox(const XMVECTOR& halfExtents) const;

	/*	Creates an entity with a plane mesh and rigid body with extents equal to input's x
		and z components.
		The plane's position will be at 0,0,0, and the rotation will be identity.
		Scale will be based on input parameter. y and w components are ignored.
		The plane's normal will be 0,1,0 (pointing straight up).
		Planes will be created as fixed, you should not try to make them dynamic.
	*/
	bsEntity* createPlane(const XMVECTOR& halfExtents) const;

	/*	Creates an entity with a capsule mesh and rigid body.

		The height describes the total height of the capsule, while radius describes the
		radius of the two spheres at the bottom and top of the capsule (and the cylinder
		between them).

		The height between the center of the capsule's two spheres must always be larger
		than 2*radius. This is because the difference between the two sphere centers must
		always be positive.

		The graphical representation of the two spheres on either end of the capsule will
		not be completely aligned with the physical version when the ratio between height
		and radius differ much from the original mesh.
		Because of this, you will get best results when height is approximately 4 times
		larger than the radius (graphical and physical versions will match 100% at this
		ratio).
	*/
	bsEntity* createCapsule(float height, float radius) const;

	/*	Creates a cylinder with the provided height and radius.
		
		The height describes the total height of the cylinder. The lowest part of the
		cylinder will always be a circle with center (0,0,0) in local space, while the
		highest part of it will be a circle with center (0,height,0).

		Height and radius must always be positive.

		Dynamic cylinders do have artifacts when moving, as they may be simulated as
		capsules depending on the environment, so edges may clip with other objects.
		For more info, see Physics\Collide\Shape\Convex\Cylinder\hkpCylinderShape.h
	*/
	bsEntity* createCylinder(float height, float radius) const;


private:
	bsPrimitiveCreator& operator=(const bsPrimitiveCreator&);

	bsMeshCache&	mMeshCache;
	bsTextureCache&	mTextureCache;
};
