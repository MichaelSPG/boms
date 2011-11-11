#include "StdAfx.h"

#include "bsGeometryUtils.h"

#include <Physics/Collide/Shape/Convex/hkpConvexShape.h>
#include <Physics/Collide/Shape/Convex/Sphere/hkpSphereShape.h>
#include <Physics/Utilities/VisualDebugger/Viewer/hkpShapeDisplayBuilder.h>
#include <Common/Visualize/hkDisplayGeometryBuilder.h>
#include <Common/Visualize/Shape/hkDisplayGeometry.h>
#include <Common/Visualize/Shape/hkDisplaySphere.h>
#include <Common/Base/Types/Geometry/hkGeometry.h>
#include <Common/Visualize/hkDebugDisplay.h>
#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>
#include <Common/Base/Types/Geometry/Sphere/hkSphere.h>
#include <Common/Visualize/Shape/hkDisplayCylinder.h>
#include <Common/Visualize/Shape/hkDisplayAABB.h>
#include <Common/Visualize/Shape/hkDisplayGeometry.h>
#include <Common/Visualize/hkDebugDisplay.h>
#include <Common/Visualize/Shape/hkDisplayBox.h>

#include "bsLineRenderer.h"
#include "bsConvert.h"
#include "bsAssert.h"

namespace bsGeometry
{
void createLinesFromShape(const hkpShape* const shape, bsLineRenderer* line3D)
{
	//BS_ASSERT(shape->getType() == HK_SHAPE_BOX);

	//Create the geometry
	hkArray<hkDisplayGeometry*> geometry;
	hkpShapeDisplayBuilder::hkpShapeDisplayBuilderEnvironment environment;
	hkpShapeDisplayBuilder builder(environment);
	builder.buildShapeDisplay(shape, hkTransform::getIdentity(), geometry);

	BS_ASSERT(geometry.getSize() == 1);

	hkArray<hkVector4> lines;
	geometry[0]->getWireframeGeometry(lines);

	const int count = lines.getSize();
	//BS_ASSERT(count);

	//Copy the points
	std::vector<XMFLOAT3> points(count);
	for (int i = 0; i < count; ++i)
	{
		XMVECTOR point = bsMath::toXM(lines[i]);
		XMStoreFloat3(&points[i], point);
		//points[i] = bsMath::toXM3(lines[i]);
	}

	line3D->addPoints(&points[0], count);


	/*
	hkAabb aabb;
	const hkpSphereShape* sphere = static_cast<const hkpSphereShape*>(shape);
	hkArray<hkDisplayGeometry*> sphereGeom;
	builder.buildShapeDisplay_Sphere(sphere, hkTransform::getIdentity(), sphereGeom);
	hkArray<hkDisplaySphere*> sphereGeom2(sphereGeom.getSize());
	for (int i = 0; i < sphereGeom.getSize(); ++i)
	{
		sphereGeom2[i] = static_cast<hkDisplaySphere*>(sphereGeom[i]);
	}

	//auto preGeom = sphereGeom2[0]->getWireframeGeometry(lines, )
	auto preGeom = sphereGeom2[0]->getGeometry();
	sphereGeom2[0]->buildGeometry();
	auto postGeom = sphereGeom2[0]->getGeometry();

	hkSphere s;
	sphereGeom2[0]->getSphere(s);

	hkDisplaySphere ds(s, 4, 4);
	ds.buildGeometry();
	auto dsGeom = ds.getGeometry();
	hkArray<hkVector4> wflines;
	ds.getWireframeGeometry(wflines, hkContainerHeapAllocator().get(&wflines));
	
	

	hkArray<hkVector4> sphereLines;
	sphereGeom[0]->getWireframeGeometry(sphereLines);

	const int sphereCount = sphereLines.getSize();

	std::vector<XMFLOAT3> sphereLinesXM(sphereCount);
	for (int i = 0; i < sphereCount; ++i)
	{
		sphereLinesXM[i] = bsMath::toXM(sphereLines[i]);
	}

	line3D->addPoints(sphereLinesXM);
	*/
}

void createLinesFromSphere(const XMFLOAT3& sphereCenter, float radius,
	unsigned int linesPerAxis, bsLineRenderer* lineRendererOut)
{
	std::vector<XMFLOAT3> points;
	// 3=XYZ axes, 2=start and end points for each line.
	points.reserve(linesPerAxis * 3 * 2);

	//How much to step with per loop iteration so that currentDelta will interpolate
	//between 0 and 2*pi.
	const float delta = (XM_PI * 2) / linesPerAxis;

	float currentDelta = 0.0f;


	/// Generate circle around X axis.
	//The first point needs to be stored so that the last line can connect to it.
	XMFLOAT3 firstPoint(sphereCenter.x,
		(sin(currentDelta) * radius) + sphereCenter.y,
		(-cos(currentDelta) * radius) + sphereCenter.z);
	//Point being used in the loop to generate interpolated points.
	XMFLOAT3 point;

	{
		points.push_back(firstPoint);
		currentDelta += delta;

		for (unsigned int i = 1; i < linesPerAxis; ++i, currentDelta += delta)
		{
			point.x = sphereCenter.x;
			point.y = (sin(currentDelta) * radius) + sphereCenter.y;
			point.z = (-cos(currentDelta) * radius) + sphereCenter.z;

			points.push_back(point);
			points.push_back(point);
		}
		points.push_back(firstPoint);
	}


	/// Generate circle around Y axis.
	{
		currentDelta = 0.0f;

		firstPoint.x = (sin(currentDelta) * radius) + sphereCenter.x;
		firstPoint.y = sphereCenter.y;
		firstPoint.z = (-cos(currentDelta) * radius) + sphereCenter.z;
		points.push_back(firstPoint);
		currentDelta += delta;

		for (unsigned int i = 1; i < linesPerAxis; ++i, currentDelta += delta)
		{
			point.x = (sin(currentDelta) * radius) + sphereCenter.x;
			point.y = sphereCenter.y;
			point.z = (-cos(currentDelta) * radius) + sphereCenter.z;

			points.push_back(point);
			points.push_back(point);
		}
		points.push_back(firstPoint);
	}


	/// Generate circle around Z axis.
	{
		currentDelta = 0.0f;

		firstPoint.x = (sin(currentDelta) * radius) + sphereCenter.x;
		firstPoint.y = (-cos(currentDelta) * radius) + sphereCenter.y;
		firstPoint.z = sphereCenter.z;
		points.push_back(firstPoint);
		currentDelta += delta;

		for (unsigned int i = 1; i < linesPerAxis; ++i, currentDelta += delta)
		{
			point.x = (sin(currentDelta) * radius) + sphereCenter.x;
			point.y = (-cos(currentDelta) * radius) + sphereCenter.y;
			point.z = sphereCenter.z;

			points.push_back(point);
			points.push_back(point);
		}
		points.push_back(firstPoint);
	}


	lineRendererOut->addPoints(points.data(), points.size());
}

void createLinesFromBox(const XMFLOAT3& halfExtents, bsLineRenderer* lineRendererOut)
{
	const XMFLOAT3 corners[8] =
	{
		XMFLOAT3(-halfExtents.x, -halfExtents.y, -halfExtents.z), //Left bottom back
		XMFLOAT3(-halfExtents.x, -halfExtents.y,  halfExtents.z), //Left bottom front
		XMFLOAT3(-halfExtents.x,  halfExtents.y, -halfExtents.z), //Left top back
		XMFLOAT3(-halfExtents.x,  halfExtents.y,  halfExtents.z), //Left top front

		XMFLOAT3( halfExtents.x, -halfExtents.y, -halfExtents.z), //Right bottom back
		XMFLOAT3( halfExtents.x, -halfExtents.y,  halfExtents.z), //Right bottom front
		XMFLOAT3( halfExtents.x,  halfExtents.y, -halfExtents.z), //Right top back
		XMFLOAT3( halfExtents.x,  halfExtents.y,  halfExtents.z), //Right top front
	};

	const XMFLOAT3 lines[24] =
	{
		//Top
		corners[2], corners[6], //Left back - right back
		corners[2], corners[3], //Left back - left front
		corners[3], corners[7], //Left front - right front
		corners[7], corners[6], //Right front - right back

		//Bottom
		corners[0], corners[4], //Left back - right back
		corners[0], corners[1], //Left back - left front
		corners[1], corners[5], //Left front - right front
		corners[5], corners[4], //Right front - right back

		//Top to bottom
		corners[2], corners[0], //Left back
		corners[6], corners[4], //Right back
		corners[3], corners[1], //Left front
		corners[7], corners[5], //Right front
	};

	lineRendererOut->addPoints(lines, 24);
}

void createLinesFromCylinder(float height, float radius, unsigned int numSides,
	bsLineRenderer* lineRendererOut)
{
	std::vector<XMFLOAT3> points;
	//6 = the number of points needed per side (1 line on each of top/bottom lines + 
	//1 line to connect the points on top/bottom lines).
	points.reserve(numSides * 6);

	//How much to step with per loop iteration so that currentDelta will interpolate
	//between 0 and 2*pi.
	const float delta = (XM_PI * 2) / numSides;


	/// Generate circle around bottom Y axis.
	{
		float currentDelta = 0.0f;

		//The first point needs to be stored so that the last line can connect to it.
		const XMFLOAT3 firstPoint(sin(currentDelta) * radius, 0.0f,
			-cos(currentDelta) * radius);

		points.push_back(firstPoint);
		currentDelta += delta;

		XMFLOAT3 previousPoint(firstPoint);

		//Point being used to generate interpolated points.
		XMFLOAT3 point;

		for (unsigned int i = 1; i < numSides; ++i, currentDelta += delta)
		{
			//Point on bottom circle.
			point.x = sin(currentDelta) * radius;
			point.y = 0.0f;
			point.z = -cos(currentDelta) * radius;

			//End of previous line segment going around bottom circle.
			points.push_back(point);
			{
				//Line segment going from a point on the bottom circle to the equivalent
				//point on the top circle.
				XMFLOAT3 topPt(point);
				topPt.y = height;
				points.push_back(point);
				points.push_back(topPt);

				//Segment of circle going around top circle.
				XMFLOAT3 previousTopPt(previousPoint);
				previousTopPt.y = height;
				points.push_back(previousTopPt);
				points.push_back(topPt);
			}
			//Start new line segment going around bottom circle.
			points.push_back(point);

			previousPoint = point;
		}
		points.push_back(firstPoint);


		//Final line segment of top circle to close it.
		XMFLOAT3 topFirst(firstPoint);
		topFirst.y = height;
		XMFLOAT3 topLast(point);
		topLast.y = height;

		points.push_back(topLast);
		points.push_back(topFirst);

		//Final vertical line going from bottom start point to equivalent point on top circle.
		points.push_back(topFirst);
		points.push_back(firstPoint);
	}


	lineRendererOut->addPoints(points.data(), points.size());
}

} // namespace bsGeometry
