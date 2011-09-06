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

#include "bsLine3D.h"
#include "bsConvert.h"
#include "bsAssert.h"

namespace bsGeometryUtils
{
void createLinesFromShape(const hkpShape* const shape, bsLine3D* line3D)
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
		points[i] = bsMath::toXM3(lines[i]);
	}

	line3D->addPoints(points);


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

} // namespace bsGeometryUtils