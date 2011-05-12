#ifndef BS_GEOMETRYUTILS_H
#define BS_GEOMETRYUTILS_H

#include "bsConfig.h"

#include <Common/Base/hkBase.h>
#include <Physics/Collide/Shape/hkpShape.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>

class bsLine3D;


/*	Geometric utilities used to create geometry Havok objects.
	Not yet functional.
*/
namespace bsGeometryUtils
{
void createLinesFromShape(const hkpShape* const shape, bsLine3D* line3D);

void createLinesFromAabb(const hkAabb& aabb, bsLine3D* line3D)
{
	hkVector4 halfExtents;
	aabb.getHalfExtents(halfExtents);
	hkpBoxShape* shape = new hkpBoxShape(halfExtents);
	createLinesFromShape(shape, line3D);
	delete shape;
}

};

#endif // BS_GEOMETRYUTILS_H