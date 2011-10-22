#pragma once


#include <Common/Base/hkBase.h>
#include <Physics/Collide/Shape/hkpShape.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>

class bsLine3D;


/*	Geometric utilities used to create geometry Havok objects.
	Not yet functional.
*/
namespace bsGeometry
{
void createLinesFromShape(const hkpShape* const shape, bsLine3D* line3D);

/*	Creates lines to visualize a sphere. The lines will form circles around x, y and z
	axes. Each axis' line will have as many lines as specified by the linesPerAxis parameter.
	
	The total amount of lines will be linesPerAxis * 3.

	This function will NOT call build() for the line renderer.
*/
void createLinesFromSphere(const XMFLOAT3& sphereCenter, float radius,
	unsigned int linesPerAxis, bsLine3D* lineRendererOut);

} // namespace bsGeometry
