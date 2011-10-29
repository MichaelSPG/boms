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

} // namespace bsGeometry
