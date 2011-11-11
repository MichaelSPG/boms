#pragma once

#include <Common/Base/hkBase.h>
#include <Physics/Collide/Shape/hkpShape.h>

#include <xnamath.h>

class bsLineRenderer;


/*	Geometric utilities used to create geometry Havok objects.
	Not yet functional.

	These functions do NOT call build() on the provided line renderers, they only add
	a number of points to them.
*/
namespace bsGeometry
{
void createLinesFromShape(const hkpShape* const shape, bsLineRenderer* line3D);

/*	Creates lines to visualize a sphere. The lines will form circles around x, y and z
	axes. Each axis' line will have as many lines as specified by the linesPerAxis parameter.
	
	The total amount of lines will be linesPerAxis * 3.

	This function will NOT call build() for the line renderer.
*/
void createLinesFromSphere(const XMFLOAT3& sphereCenter, float radius,
	unsigned int linesPerAxis, bsLineRenderer* lineRendererOut);

void createLinesFromBox(const XMFLOAT3& halfExtents, bsLineRenderer* lineRendererOut);

/*	Creates lines to visualize a cylinder. The cylinder's origin will be at 0,0,0.
	numSides describes the cylinder's tesselation, and its value will increase
	both the number of sides of the cylinder and the top/bottom circles' amount of detail.
	
*/
void createLinesFromCylinder(float height, float radius, unsigned int numSides,
	bsLineRenderer* lineRendererOut);

} // namespace bsGeometry
