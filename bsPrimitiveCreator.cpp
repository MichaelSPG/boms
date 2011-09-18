#include "StdAfx.h"

#include "bsPrimitiveCreator.h"

#include <Physics/Collide/Shape/hkpShape.h>
#include <Physics/Collide/Shape/Convex/Sphere/hkpSphereShape.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics/Collide/Shape/HeightField/Plane/hkpPlaneShape.h>

#include "bsSceneNode.h"
#include "bsMeshCache.h"
#include "bsMath.h"


bsSceneNode* bsPrimitiveCreator::createSphere(float radius)
{
	bsSceneNode* sceneNode = new bsSceneNode();
	bsEntity& entity = sceneNode->getEntity();
	entity.attach(mMeshCache.getMesh("sphere_1m_d.bsm"));

	hkpRigidBodyCinfo rbCinfo;
	rbCinfo.m_shape = new hkpSphereShape(radius);
	rbCinfo.m_motionType = hkpMotion::MOTION_SPHERE_INERTIA;

	entity.attach(new hkpRigidBody(rbCinfo));

	sceneNode->setLocalScaleUniform(radius * 2.0f);

	return sceneNode;
}

bsSceneNode* bsPrimitiveCreator::createBox(const XMVECTOR& halfExtents)
{
	bsSceneNode* sceneNode = new bsSceneNode();
	bsEntity& entity = sceneNode->getEntity();
	entity.attach(mMeshCache.getMesh("unit_cube.bsm"));

	hkVector4 halfExt(bsMath::toHK(halfExtents));
	//Subtract convex radius from the half extents to prevent the box from
	//appearing to hover over the ground.
	const hkVector4 convexRadius(hkConvexShapeDefaultRadius, hkConvexShapeDefaultRadius,
		hkConvexShapeDefaultRadius);
	halfExt.sub(convexRadius);

	hkpRigidBodyCinfo rbCinfo;
	rbCinfo.m_shape = new hkpBoxShape(halfExt);
	rbCinfo.m_motionType = hkpMotion::MOTION_BOX_INERTIA;

	entity.attach(new hkpRigidBody(rbCinfo));

	sceneNode->setLocalScale(XMVectorScale(halfExtents, 2.0f));

	return sceneNode;
}

bsSceneNode* bsPrimitiveCreator::createPlane(const XMVECTOR& halfExtents)
{
	bsSceneNode* sceneNode = new bsSceneNode();
	bsEntity& entity = sceneNode->getEntity();
	entity.attach(mMeshCache.getMesh("plane_1m.bsm"));

	hkVector4 halfExt;
	halfExt.load<4>(halfExtents.m128_f32);

	hkpRigidBodyCinfo rbCinfo;
	//Create plane shape point upwards at origin.
	rbCinfo.m_shape = new hkpPlaneShape(hkVector4::getConstant<HK_QUADREAL_0100>(), hkVector4::getZero(), halfExt);
	rbCinfo.m_motionType = hkpMotion::MOTION_FIXED;

	entity.attach(new hkpRigidBody(rbCinfo));

	XMVECTOR scale = XMVectorScale(halfExtents, 2.0f);
	//Set Y scale to 1 regardless of input.
	scale = XMVectorSetY(scale, 1.0f);
	sceneNode->setLocalScale(scale);

	return sceneNode;
}